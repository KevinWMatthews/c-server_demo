#ifndef SOCKET_UNIX_INCLUDED
#define SOCKET_UNIX_INCLUDED

#include <stddef.h>

#define SOCKETFD_INVALID    -1

void unlink_file(const char *file);
void close_file_descriptor(int fd);
int unix_listen(const char *socket_filename);
int unix_connect(int local_socket, const char *socket_filename);
int unix_socket_receive(int socket, char *buffer, size_t buffer_len);
int transmit_data(int socket_fd, const char *buffer);

#endif
