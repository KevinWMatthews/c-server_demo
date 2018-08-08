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
        char buffer[1024] = {0};
        ret = unix_socket_receive(listen_socket, buffer, sizeof(buffer));
        if (ret < 0)
            continue;

        printf("Received data:\n%s\n", buffer);
        // Could send data to client
    }

    return 0;
}
