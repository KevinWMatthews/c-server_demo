#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define SOCKETFD_INVALID    -1

// Returns the socket file descriptor on success (>= 0)
// Returns -1 on failure.
int unix_listen(unsigned int port)
{
    int socket_fd = SOCKETFD_INVALID;

    socket_fd = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        perror("Failed to create socket");
        return SOCKETFD_INVALID;
    }

    return socket_fd;
}

int main(void)
{
    int listen_socket = SOCKETFD_INVALID;

    printf("Starting server...\n");
    // Open a socket to listen for new connection requests.
    listen_socket = unix_listen(9191);
    if (listen_socket < 0)
    {
        // If we can't listen for new connections, we have nothing to do.
        fprintf(stderr, "Failed to listen on socket. Exiting.\n");
        exit(EXIT_FAILURE);
    }
    printf("Done\n");

    if ( close(listen_socket) < 0 )
    {
        perror("Failed to close listen socket on shutdown");
        exit(EXIT_FAILURE);
    }
    return 0;
}
