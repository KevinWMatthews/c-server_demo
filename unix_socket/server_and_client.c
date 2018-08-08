#define _GNU_SOURCE

#include "socket_unix.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static void *client_thread(void *args)
{
    while (1)
        ;
    return NULL;
}

int main(void)
{
    pthread_t client_handle = {0};
    pthread_t client_handle2 = {0};
    pthread_attr_t *client_attr = NULL;
    void *client_args = NULL;
    int ret;

    ret = pthread_create(&client_handle, client_attr, client_thread, client_args);
    if (ret != 0)
    {
        fprintf(stderr, "Unable to spawn client thread\n");
        exit(EXIT_FAILURE);
    }
    printf("Spawned client thread\n");

    ret = pthread_setname_np(client_handle, "Client Thread");
    if (ret != 0)
    {
        fprintf(stderr, "Error assigning name to client thread\n");
    }

    ret = pthread_create(&client_handle2, client_attr, client_thread, client_args);
    if (ret != 0)
    {
        fprintf(stderr, "Unable to spawn client thread\n");
        exit(EXIT_FAILURE);
    }
    printf("Spawned client thread\n");

    ret = pthread_setname_np(client_handle2, "Client Thread2");
    if (ret != 0)
    {
        fprintf(stderr, "Error assigning name to client thread\n");
    }
    while (1)
        ;
    return 0;
}
