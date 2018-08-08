#include "socket_unix.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>

void unlink_file(const char *file)
{
    if ( unlink(file) < 0 )
    {
        perror("Failed to unlink file");
        fprintf(stderr, "Filename: %s\n", file);
    }
}

void close_file_descriptor(int fd)
{
    if ( close(fd) < 0 )
    {
        perror("Failed to close file");
        fprintf(stderr, "File descriptor: %d\n", fd);
    }
}

static int copy_unix_socket_filename(struct sockaddr_un *addr, const char *filename)
{
    if (addr == NULL)
    {
        fprintf(stderr, "Socket address can not be NULL\n");
        return -1;
    }
    if (filename == NULL)
    {
        fprintf(stderr, "Socket filename can not be NULL.\n");
        return -1;
    }

    size_t length = strlen(filename);
    if (length >= sizeof(addr->sun_path)-1)
    {
        fprintf(stderr, "Server socket filename is too long: %s\n", filename);
        fprintf(stderr, "Is (%zu) characters but can not be greater than (%zu)\n", length, sizeof(addr->sun_path)-1);
        return -1;
    }

    addr->sun_family = AF_UNIX;
    strncpy(addr->sun_path, filename, length);

    return 0;
}

// Returns the socket file descriptor on success (>= 0)
// Returns -1 on failure.
int unix_listen(const char *socket_filename)
{
    int socket_fd = SOCKETFD_INVALID;
    struct sockaddr_un addr = {0};
    int ret;

    printf("Creating socket file: %s\n", socket_filename);

    ret = copy_unix_socket_filename(&addr, socket_filename);
    if (ret < 0)
    {
        fprintf(stderr, "Invalid socket filename\n");
        return SOCKETFD_INVALID;
    }

    socket_fd = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        perror("Failed to create socket");
        return SOCKETFD_INVALID;
    }

    // "Everybody else does it this way" - leaving a file behind typically isn't so bad
    // Try it the hard way to see what I learn.
    // unlink(/* socket_file */);
    ret = bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        perror("Failed to bind socket");
        return SOCKETFD_INVALID;
    }

    return socket_fd;
}

// Connect to a remote socket
// Returns 0 on success, -1 on failure
int unix_connect(int local_socket, const char *socket_filename)
{
    struct sockaddr_un remote_addr = {0};
    socklen_t remote_addr_len = sizeof(remote_addr);
    int ret;

    remote_addr.sun_family = AF_UNIX;
    ret = copy_unix_socket_filename(&remote_addr, socket_filename);
    if (ret < 0)
    {
        fprintf(stderr, "Invalid socket filename\n");
        return -1;
    }

    ret = connect(local_socket, (struct sockaddr *)&remote_addr, remote_addr_len);
    if (ret < 0)
    {
        perror("Failed to connect to socket");
        return -1;
    }

    return 0;
}

int unix_socket_receive(int socket, char *buffer, size_t buffer_len)
{
    if (buffer == NULL)
    {
        fprintf(stderr, "%s: buffer can not be null\n", __func__);
        return -1;
    }
    //TODO buffer_len restriction?

    struct sockaddr_un from = {0};
    socklen_t from_length = sizeof(from);
    int flags = 0;      // Anything useful here?
    int len = 0;

    // Could use:
    //  len = recv(listen_socket, buffer, sizeof(buffer), flags);
    //  but this is typically used for a connected socket.
    len = recvfrom(socket, buffer, buffer_len, flags, (struct sockaddr *)&from, &from_length);
    if (len < 0)
    {
        perror("Failed to read from socket");
        return -1;
    }
    else if (len == 0)
    {
        ;       // 0-length datagrams are permitted (see `man 2 recvfrom`)
    }
    //TODO parse address from sockaddr

    return 0;
}

int unix_socket_transmit(int socket_fd, const char *buffer, size_t buffer_len)
{
    ssize_t ret;
    int flags = 0;   // Something cool in here??

    if (buffer == NULL)
    {
        fprintf(stderr, "%s: Buffer may not be null\n", __func__);
        return -1;
    }
    //TODO buffer_len restriction?

    ret = send(socket_fd, buffer, buffer_len, flags);
    if (ret < 0)
    {
        perror("Failed to send");
        return -1;
    }
    return 0;
}
