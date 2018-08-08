#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define SOCKETFD_INVALID    -1
#define CLIENT_SOCKET_FILE  "./client_socket_file"
#define SERVER_SOCKET_FILE  "./server_socket_file"      //TODO parse this from the command line

static int local_socket = SOCKETFD_INVALID;

static void unlink_file(const char *file)
{
    if ( unlink(file) < 0 )
    {
        perror("Failed to unlink file");
        fprintf(stderr, "Filename: %s\n", file);
    }
}

static void close_file(int fd)
{
    if ( close(fd) < 0 )
    {
        perror("Failed to close file");
        fprintf(stderr, "File descriptor: %d\n", fd);
    }
}

static void execute_at_exit(void)
{
    fprintf(stdout, "Tearing down socket...\n");
    unlink_file(CLIENT_SOCKET_FILE);
    close_file(local_socket);
    fprintf(stdout, "Done\n");
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
static int unix_listen(const char *socket_filename)
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
static int unix_connect(int local_socket, const char *socket_filename)
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

static int transmit_data(int socket_fd, const char *buffer)
{
    ssize_t ret;

    if (buffer == NULL)
    {
        fprintf(stderr, "%s: Buffer may not be null\n", __func__);
        return -1;
    }

    size_t buffer_len = strlen(buffer) + 1;     // Add the null terminator
    printf("Sending message (%zu): %s\n", buffer_len, buffer);
    ret = send(socket_fd, buffer, buffer_len, 0);   // Flags?
    if (ret < 0)
    {
        perror("Failed to send");
        return -1;
    }
    return 0;
}

int main(void)
{
    const char buffer[] = {"This is from a client"};
    int ret;

    ret = atexit(execute_at_exit);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to set exit handler\n");
        exit(EXIT_FAILURE);
    }

    local_socket = unix_listen(CLIENT_SOCKET_FILE);
    if (local_socket < 0)
    {
        // If we can't listen for new connections, we have nothing to do.
        fprintf(stderr, "Failed to create socket. Exiting.\n");
        exit(EXIT_FAILURE);
    }
    printf("Done.\n");

    printf("Connecting to server socket\n");
    ret = unix_connect(local_socket, SERVER_SOCKET_FILE);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to connect to socket. Exiting\n");
        exit(EXIT_FAILURE);
    }

    ret = transmit_data(local_socket, buffer);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to transmit data. Exiting\n");
        exit(EXIT_FAILURE);
    }

    // Could receive data here

    return 0;
}
