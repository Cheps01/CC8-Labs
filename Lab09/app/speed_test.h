#ifndef SPEED_TEST_H
#define SPEED_TEST_H

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

int local_ip_for(struct in_addr dst, struct in_addr *out);
int send_segment(conn_t *c, uint32_t seq, uint32_t ack, uint8_t flags,
                        const void *data, size_t len);
int wait_for(conn_t *c, uint8_t need, struct tcphdr *hdr,
                    const uint8_t **payload, size_t *plen, int64_t *t_us);

#endif