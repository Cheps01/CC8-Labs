#include "speed_test.h"
 
// ---------------------------------
//          UTILIDADES
// ---------------------------------
int64_t now_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}
 
// Checksum de Internet
static uint16_t csum16(const void *data, size_t len) {
    const uint8_t *p = data;
    uint32_t sum = 0;
    while (len > 1) {
        uint16_t w;
        memcpy(&w, p, 2);
        sum += w;
        p += 2;
        len -= 2;
    }
    if (len) {
        uint16_t w = 0;
        memcpy(&w, p, 1);
        sum += w;
    }
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);
    return (uint16_t)~sum;
}
 
// Checksum TCP
static uint16_t tcp_checksum(struct in_addr src, struct in_addr dst,
                             const void *seg, size_t seg_len) {
    struct __attribute__((packed)) {
        uint32_t src, dst;
        uint8_t zero, proto;
        uint16_t len;
    } ph = {src.s_addr, dst.s_addr, 0, IPPROTO_TCP, htons((uint16_t)seg_len)};
 
    uint8_t buf[sizeof ph + MAX_PKT] __attribute__((aligned(4)));
    memcpy(buf, &ph, sizeof ph);
    memcpy(buf + sizeof ph, seg, seg_len);
    return csum16(buf, sizeof ph + seg_len);
}
 
// Descubre que IP local usaria el kernel para llegar a dst
int local_ip_for(struct in_addr dst, struct in_addr *out) {
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
        return -1;
    struct sockaddr_in a = {.sin_family = AF_INET, .sin_port = htons(9), .sin_addr = dst};
    if (connect(s, (struct sockaddr *)&a, sizeof a) < 0) {
        close(s);
        return -1;
    }
    struct sockaddr_in me;
    socklen_t l = sizeof me;
    if (getsockname(s, (struct sockaddr *)&me, &l) < 0) {
        close(s);
        return -1;
    }
    close(s);
    *out = me.sin_addr;
    return 0;
}
 
// ---------------------------------
//  ENVIO Y RECEPCIÓN DE SEGMENTOS
// ---------------------------------

// Envia un segmento TCP. Devuelve 0 si ok, -1 si error.
int send_segment(conn_t *c, uint32_t seq, uint32_t ack, uint8_t flags,
                        const void *data, size_t len) {
    uint8_t buf[MAX_PKT] __attribute__((aligned(4)));
    size_t hlen = sizeof(struct tcphdr);
    if (hlen + len > sizeof buf)
        return -1;
 
    struct tcphdr *th = (struct tcphdr *)buf;
    memset(th, 0, hlen);
    th->th_sport = htons(c->sport);
    th->th_dport = htons(c->dport);
    th->th_seq = htonl(seq);
    th->th_ack = htonl(ack);
    th->th_off = hlen / 4;
    th->th_flags = flags;
    th->th_win = htons(65535);
    if (len)
        memcpy(buf + hlen, data, len);
    th->th_sum = 0;
    th->th_sum = tcp_checksum(c->src_ip, c->dst_ip, buf, hlen + len);
 
    struct sockaddr_in to = {.sin_family = AF_INET, .sin_addr = c->dst_ip};
    ssize_t n = sendto(c->sock, buf, hlen + len, 0, (struct sockaddr *)&to, sizeof to);
    return n == (ssize_t)(hlen + len) ? 0 : -1;
}
 
// Espera un segmento del servidor dirigido a nuestro puerto. Devuelve 0 si ok, -1 por timeout o error.
int wait_for(conn_t *c, uint8_t need, struct tcphdr *hdr,
                    const uint8_t **payload, size_t *plen, int64_t *t_us) {
    static uint8_t rbuf[MAX_PKT] __attribute__((aligned(4)));
    int64_t deadline = now_us() + (int64_t)WAIT_MS * 1000;
 
    for (;;) {
        int64_t left = deadline - now_us();
        if (left <= 0)
            return -1;
        struct pollfd pfd = {.fd = c->sock, .events = POLLIN};
        if (poll(&pfd, 1, (int)(left / 1000) + 1) <= 0)
            return -1;
 
        ssize_t n = recv(c->sock, rbuf, sizeof rbuf, 0);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        int64_t t = now_us();
 
        if (n < (ssize_t)sizeof(struct iphdr))
            continue;
        struct iphdr *ip = (struct iphdr *)rbuf;
        size_t ihl = (size_t)ip->ihl * 4;
        if (ip->protocol != IPPROTO_TCP || ip->saddr != c->dst_ip.s_addr)
            continue;
        if ((size_t)n < ihl + sizeof(struct tcphdr))
            continue;
 
        struct tcphdr *th = (struct tcphdr *)(rbuf + ihl);
        if (ntohs(th->th_sport) != c->dport || ntohs(th->th_dport) != c->sport)
            continue;
        if ((th->th_flags & need) != need)
            continue;
 
        size_t thl = (size_t)th->th_off * 4;
        if (thl < sizeof(struct tcphdr) || ihl + thl > (size_t)n)
            continue;
 
        *hdr = *th;
        *payload = rbuf + ihl + thl;
        *plen = (size_t)n - ihl - thl;
        *t_us = t;
        return 0;
    }
}