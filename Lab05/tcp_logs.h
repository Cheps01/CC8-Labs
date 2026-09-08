#ifndef TCP_LOGS_H
#define TCP_LOGS_H

void log_divider(const char* title);
void log_packet(int is_client, uint8_t flags, uint32_t seq,uint32_t ack, const char* data);

#endif