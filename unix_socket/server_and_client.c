#define _GNU_SOURCE

#include "socket_unix.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

// A "data key".
// It is visible to all threads, but data that is bound to it is distinct for each thread.
// Each thread binds thread-specific data to the key.
// When a thread retrieves data from the key, only its data is accessible.
// Used to implement a thread-specific destructor.
static pthread_key_t key;

static void *client_thread(void *args)
{
    void * ptr = NULL;
    int ret;

    ptr = pthread_getspecific(key);
    if (ptr != NULL)
    {
        fprintf(stderr, "%s: Key has already been bound!\n", __func__);
        return NULL;
    }
    ptr = malloc(sizeof(int));
    *(int *)ptr = 42;
    ret = pthread_setspecific(key, ptr);
    if (ret != 0)
    {
        fprintf(stderr, "%s: Failed to bind thread-specific data to key\n", __func__);
        free(ptr);
        ptr = NULL;
        return NULL;
    }

    void *p2;
    p2 = pthread_getspecific(key);
    printf("ret: %d\n", *(int *)p2);
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

    // Initialize the pthread key.
    // Can use pthread_once() to do this once, or do this before any threads are created.
    ret = pthread_key_create(&key, NULL);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to create pthread key: %d\n", ret);
        exit(EXIT_FAILURE);
    }

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
