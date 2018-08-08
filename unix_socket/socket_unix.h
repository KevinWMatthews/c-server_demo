#ifndef SOCKET_UNIX_INCLUDED
#define SOCKET_UNIX_INCLUDED

#include <stddef.h>

#define SOCKETFD_INVALID    -1

void unlink_file(const char *file);
void close_file_descriptor(int fd);

// Returns socket descriptor on success (>= 0), SOCKETFD_INVALID (< 0) on failure.
int unix_socket_listen(const char *socket_filename);

// Connect to a remote socket
// Returns 0 on success, -1 on failure
int unix_socket_connect(int local_socket, const char *socket_filename);

// String in buffer will be null terminated?
int unix_socket_receive(int socket, char *buffer, size_t buffer_len);

// String in buffer must be null terminated.
int unix_socket_transmit(int socket_fd, const char *buffer, size_t buffer_len);

#endif
