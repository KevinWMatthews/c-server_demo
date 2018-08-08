#ifndef SOCKET_UNIX_INCLUDED
#define SOCKET_UNIX_INCLUDED

#define SOCKETFD_INVALID    -1

void unlink_file(const char *file);
void close_file_descriptor(int fd);
int unix_listen(const char *socket_filename);
int unix_connect(int local_socket, const char *socket_filename);
void handle_incomming_data(int listen_socket);
int transmit_data(int socket_fd, const char *buffer);

#endif
