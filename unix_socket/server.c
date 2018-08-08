#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "socket_unix.h"

#define SERVER_SOCKET_FILE  "./server_socket_file"

// Make listen socket file-scope for easy teardown with atexit()
static int listen_socket = SOCKETFD_INVALID;


static void execute_at_exit(void)
{
    fprintf(stdout, "Tearing down socket...\n");
    unlink_file(SERVER_SOCKET_FILE);
    close_file_descriptor(listen_socket);
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

//TODO may be able to extract this
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
    listen_socket = unix_listen(SERVER_SOCKET_FILE);
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
        // Could send data to client
    }

    return 0;
}
