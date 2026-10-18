#ifndef RAW_SOCKET_H
#define RAW_SOCKET_H

#include <arpa/inet.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int raw_socket_open(int protocol);
ssize_t raw_socket_send(int fd, const char *dst_ip, const void *data, size_t length);
ssize_t raw_socket_read(int fd, void *buffer, size_t capacity, struct sockaddr_in *source);
int raw_socket_close(int fd);

#endif