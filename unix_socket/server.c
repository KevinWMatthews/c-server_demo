#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define SOCKETFD_INVALID    -1
#define SERVER_SOCKET_FILE  "./server_socket_file"

// Make listen socket file-scope for easy teardown with atexit()
static int listen_socket = SOCKETFD_INVALID;


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
    unlink_file(SERVER_SOCKET_FILE);
    close_file(listen_socket);
    fprintf(stdout, "Done\n");
}

static void sigint_handler(int signal)
{
    printf("Caught SIGINT\n");
    exit(EXIT_SUCCESS);     // We've registered a signal handler
}

static void sigquit_handler(int signal)
{
    printf("Caught SIGQUIT\n");
    exit(EXIT_SUCCESS);     // We've registered a signal handler
}


// Returns the socket file descriptor on success (>= 0)
// Returns -1 on failure.
static int unix_listen(unsigned int port)
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
        return SOCKETFD_INVALID;
    }

    strncpy(addr.sun_path, SERVER_SOCKET_FILE, sizeof(SERVER_SOCKET_FILE));

    // "Everybody else does it this way" - leaving a file behind typically isn't so bad
    // Try it the hard way to see what I learn.
    // unlink(SERVER_SOCKET_FILE);
    ret = bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        perror("Failed to bind socket");
        return SOCKETFD_INVALID;
    }

    return socket_fd;
}

static void handle_incomming_data(int listen_socket)
{
    char buffer[1024] = {0};
    int flags = 0;      // Anything useful here?

    do
    {
        struct sockaddr_un from = {0};
        socklen_t from_length = sizeof(from);
        int len = 0;

        // Could use:
        //  len = recv(listen_socket, buffer, sizeof(buffer), flags);
        //  but this is typically used for a connected socket.
        len = recvfrom(listen_socket, buffer, sizeof(buffer), flags, (struct sockaddr *)&from, &from_length);
        if (len < 0)
        {
            perror("Failed to read from socket");
            break;
        }
        else if (len == 0)
        {
            ;       // 0-length datagrams are permitted (see `man 2 recvfrom`)
        }
        printf("Received data:\n%s\n", buffer);
        //TODO parse address from sockaddr
    } while (1);
}

int main(void)
{
    int ret;

    // Return type is sighandler_t but I don't know where this is declared
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        perror("Failed to set SIGINT handler");
        exit(EXIT_FAILURE);
    }
    if (signal(SIGQUIT, sigquit_handler) == SIG_ERR)
    {
        perror("Failed to set SIGQUIT handler");
        exit(EXIT_FAILURE);
    }

    ret = atexit(execute_at_exit);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to set exit handler\n");
        exit(EXIT_FAILURE);
    }

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

    while (1)
    {
        handle_incomming_data(listen_socket);
    }

    return 0;
}
