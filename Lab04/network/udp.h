#ifndef UDP_H
#define UDP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void start_udp_socket(const char* server_ip, const int server_port);
void send_udp(const void *message, size_t len);
ssize_t receive_udp();

#endif // UDP_H