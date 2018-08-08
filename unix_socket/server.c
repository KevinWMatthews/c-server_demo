#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define SOCKETFD_INVALID    -1
#define SERVER_SOCKET_FILE  "./server_socket_file"

// Returns the socket file descriptor on success (>= 0)
// Returns -1 on failure.
int unix_listen(unsigned int port)
{
    int socket_fd = SOCKETFD_INVALID;
    struct sockaddr_un addr = {0};
    int ret;

    socket_fd = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        perror("Failed to create socket");
        return SOCKETFD_INVALID;
    }

    addr.sun_family = AF_UNIX;
    if (sizeof(SERVER_SOCKET_FILE) >= sizeof(addr.sun_path))
    {
        fprintf(stderr, "Server socket filename is too long\n");
        close(socket_fd);
        return -1;
    }

    strncpy(addr.sun_path, SERVER_SOCKET_FILE, sizeof(SERVER_SOCKET_FILE));
    unlink(SERVER_SOCKET_FILE);     // According to the docs you can do this at any time - when it is no longer needed.

    ret = bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        perror("Failed to bind socket");
        close(socket_fd);
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
