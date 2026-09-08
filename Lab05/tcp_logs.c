#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "tcp_logs.h"

const char* flag_names[] = {"FIN", "SYN", "RST", "PSH", "ACK", "URG"};

void log_divider(const char* title) {
    printf("------------------- %s -------------------\n", title);
}
void log_packet(int is_client, uint8_t flags, uint32_t seq, uint32_t ack, const char* data) {
    (is_client) ? printf("CLIENT -> SERVER ") : printf("SERVER -> CLIENT ");
    printf("[");
    for (int i=0; i<6; i++) {
        if (flags & 1<<i) {
            printf("%s, ", flag_names[i]);
        }
    }
    printf("] ");
    printf("SEQ=%u ", seq);
    if (ack)
        printf("ACK=%u ", ack);
    size_t len = strlen(data);
    if (len > 0) {
        printf("LEN=%ld DATA=%s", len, data);
    }
    printf("\n");
}