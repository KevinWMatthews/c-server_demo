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

// Returns the socket file descriptor on success (>= 0)
// Returns -1 on failure.
int unix_listen(const char *socket_filename)
{
    int socket_fd = SOCKETFD_INVALID;
    struct sockaddr_un addr = {0};
    int ret;

    if (socket_filename == NULL)
    {
        fprintf(stderr, "Socket filename can not be NULL.\n");
        return SOCKETFD_INVALID;
    }

    size_t filename_length = strlen(socket_filename);
    if (filename_length >= sizeof(addr.sun_path)-1)
    {
        fprintf(stderr, "Server socket filename is too long: %s\n", socket_filename);
        fprintf(stderr, "Is (%zu) characters but can not be greater than (%zu)\n", filename_length, sizeof(addr.sun_path)-1);
        return SOCKETFD_INVALID;
    }

    printf("Creating socket file: %s\n", socket_filename);

    socket_fd = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        perror("Failed to create socket");
        return SOCKETFD_INVALID;
    }

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_filename, filename_length);

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
