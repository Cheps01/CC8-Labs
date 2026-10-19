#ifndef SPEEDTEST_H
#define SPEEDTEST_H

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
 
#define F_FIN 0x01
#define F_SYN 0x02
#define F_RST 0x04
#define F_PSH 0x08
#define F_ACK 0x10
 
#define MAX_PKT 2048
#define WAIT_MS 2000
 
typedef struct {
    int sock;
    struct in_addr src_ip, dst_ip;
    uint16_t sport, dport;
} conn_t;



#endif