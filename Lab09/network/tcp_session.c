#include "tcp_session.h"

void build_ip_header(tcp_header_t *iph, uint32_t src_addr, uint32_t dst_addr, uint16_t payload_len) 
{

}
void build_tcp_header(tcp_header_t *tcph, uint16_t src_port, uint16_t dst_port, 
                    uint32_t seq_num, uint32_t ack_num, uint8_t flags, uint16_t window) 
{

}
int build_packet(const tcp_session_t *session, uint8_t *packet, uint8_t flags, 
                    const char *data, size_t data_len)
{

}
int wait_for_packet(tcp_session_t *session, ) 
{
    
}
int send_control() 
{

}
int tcp_start(tcp_session_t *session, const char *local_ip, const char *server_ip, uint16_t server_port)
{

}
int tcp_handshake(tcp_session_t *session, int timeout_sec) 
{

}
int tcp_send_message(tcp_session_t *session, const char* message)
{

}
int tcp_recv_message(tcp_session_t *session, void *buffer, size_t buff_len, int timeout_sec)
{

}
void tcp_close(tcp_session_t *session)
{

}