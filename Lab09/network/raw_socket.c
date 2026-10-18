#include "raw_socket.h"

#define _POSIX_C_SOURCE 200809L

// Open an IPv4 raw socket for the given IP protocol number
int raw_socket_open(int protocol) {
    return socket(AF_INET, SOCK_RAW, protocol);
}

// Send bytes to an IPv4 destination.
ssize_t raw_socket_send(int fd, const char *dst_ip, const void *data, size_t length) 
{
    struct sockaddr_in destination;
    if (dst_ip == NULL || (data == NULL && length != 0)) {
        errno = EINVAL;
        return -1;
    }

    memset(&destination, 0, sizeof(destination));
    destination.sin_family = AF_INET;
    if (inet_pton(AF_INET, dst_ip, &destination.sin_addr) != 1) {
        errno = EINVAL;
        return -1;
    }

    return sendto(fd, data, length, 0,
                  (const struct sockaddr *)&destination,
                  sizeof(destination));
}

// Read one incoming packet/payload.
ssize_t raw_socket_read(int fd, void *buffer, size_t capacity, struct sockaddr_in *source)
{
    struct sockaddr_in source_address;
    socklen_t source_length = sizeof(source_address);

    if (buffer == NULL || capacity == 0) {
        errno = EINVAL;
        return -1;
    }

    ssize_t received = recvfrom(fd, buffer, capacity, 0,
                                (struct sockaddr *)&source_address,
                                &source_length);

    if (received >= 0 && source != NULL) {
        *source = source_address;
    }

    return received;
}

// Close the raw socket
int raw_socket_close(int fd)
{
    return close(fd);
}