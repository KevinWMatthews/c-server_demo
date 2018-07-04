#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// Program configuration
#define SOCKET_BACKLOG      3


#define SOCKETFD_INVALID    -1


static int tcp_listen(unsigned int port)
{
    int socket_fd = SOCKETFD_INVALID;
    struct sockaddr_in sin = {0};
    int val = 1;
    int ret;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);        //TODO option 0 means...?
    if (socket_fd < 0)
    {
        perror("Failed to create socket");
        return SOCKETFD_INVALID;
    }

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);
    ret = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (ret < 0)
    {
        perror("Failed to set socket options");
        close(socket_fd);
        return SOCKETFD_INVALID;
    }

    ret = bind(socket_fd, (struct sockaddr *)&sin, sizeof(sin));
    if (ret < 0)
    {
        perror("Failed to bind socket");
        close(socket_fd);
        return SOCKETFD_INVALID;
    }

    ret = listen(socket_fd, SOCKET_BACKLOG);
    if (ret < 0)
    {
        perror("Failed to listen on socket");
        close(socket_fd);
        return SOCKETFD_INVALID;
    }

    return socket_fd;
}

static void handle_client(int socket_fd)
{
    pid_t pid = getpid();

    printf("Entering client handler: %d\n", pid);
    // Do stuff.

    printf("Exiting client handler: %d\n", pid);
    if ( close(socket_fd) < 0 )
    {
        perror("Client handler failed to close socket");
    }

    // Child processes should call _exit() instead of exit() to avoid
    // processing cleanup functions and flushing output buffers.
    // See https://www.gnu.org/software/libc/manual/html_node/Termination-Internals.html#Termination-Internals
    _exit(EXIT_SUCCESS);
}

static void handle_incoming_connections(int listen_socket)
{
    int client_socket_fd;   // Socket for the new client connection.
    pid_t pid;

    client_socket_fd = accept(listen_socket, 0, 0);     //TODO what do 0 options mean?
    if (client_socket_fd < 0)
    {
        perror("Failed to accept client connection");
        return;     // Better luck next time.
    }

    pid = fork();
    if (pid < 0)
    {
        // Error
        perror("Incoming connection failed to fork");
        close(client_socket_fd);
        return;     // Perhaps we can recover...?
    }
    else if (pid == 0)
    {
        // Child process
        handle_client(client_socket_fd);

        // I have designed the child/client process to clean up and exit when it is finished.
        // It should never return.
        fprintf(stderr, "Child process leaked!!\n");
    }
    else    // pid > 0
    {
        // Parent process
        printf("Server spawned child process: %d\n", pid);

        // Close the socket in the parent/listening process to avoid the sockets lingering in CLOSE_WAIT state
        // (you can verify this using netstat).
        //
        // The socket will still be open in the child/client process, which is where the socket is actually used.
        // Of course, the socket must also be closed in the child process.
        if ( close(client_socket_fd) < 0 )
        {
            perror("Parent process failed to close socket");
        }
    }
}

int main(void)
{
    int listen_socket = SOCKETFD_INVALID;

    printf("Starting server...\n");
    // Open a socket to listen for new connection requests.
    // Connections will be accepted on a different socket/port.
    listen_socket = tcp_listen(8484);       //TODO custom port
    if (listen_socket < 0)
    {
        // If we can't listen for new connections, we have nothing to do.
        fprintf(stderr, "Failed to listen on socket. Exiting.\n");
        exit(EXIT_FAILURE);
    }
    printf("Done\n");

    while (1)
    {
        printf("\nListening for connections...\n");
        handle_incoming_connections(listen_socket);
    }

    printf("Shutting down server...\n");
    if ( close(listen_socket) < 0 )
    {
        perror("Failed to close listen socket on shutdown");
        exit(EXIT_FAILURE);
    }

    printf("Done\n");
    return 0;
}
