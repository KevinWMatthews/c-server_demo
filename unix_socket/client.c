#include "socket_unix.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define SOCKETFD_INVALID    -1
#define CLIENT_SOCKET_FILE  "./client_socket_file"
#define SERVER_SOCKET_FILE  "./server_socket_file"      //TODO parse this from the command line

static int local_socket = SOCKETFD_INVALID;

static void execute_at_exit(void)
{
    fprintf(stdout, "Tearing down socket...\n");
    unlink_file(CLIENT_SOCKET_FILE);
    close_file_descriptor(local_socket);
    fprintf(stdout, "Done\n");
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

    local_socket = unix_socket_listen(CLIENT_SOCKET_FILE);
    if (local_socket < 0)
    {
        // If we can't listen for new connections, we have nothing to do.
        fprintf(stderr, "Failed to create socket. Exiting.\n");
        exit(EXIT_FAILURE);
    }
    printf("Done.\n");

    printf("Connecting to server socket\n");
    ret = unix_socket_connect(local_socket, SERVER_SOCKET_FILE);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to connect to socket. Exiting\n");
        exit(EXIT_FAILURE);
    }

    // strlen() or sizeof()?
    printf("Sending message (%zu): %s\n", strlen(buffer), buffer);
    ret = unix_socket_transmit(local_socket, buffer, strlen(buffer));
    if (ret < 0)
    {
        fprintf(stderr, "Failed to transmit data. Exiting\n");
        exit(EXIT_FAILURE);
    }

    // Could receive data here

    return 0;
}
