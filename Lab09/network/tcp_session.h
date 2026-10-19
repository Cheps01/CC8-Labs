#ifndef TCP_SESSION_H
#define TCP_SESSION_H

#pragma pack(push, 1);
typedef struct {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq_num;              
    uint32_t ack_num;               
    uint8_t  data_offset_reserved;  
    uint8_t  flags;                 
    uint16_t window;                
    uint16_t checksum;              
    uint16_t urgent_ptr;            
} tcp_header_t;
#pragma pack(pop)

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20

typedef struct {
    int sockfd;
    char local_ip[64];
    char server_ip[64];
    uint16_t local_port;
    uint16_t server_port;
    uint32_t seq;
    uint32_t ack;
} tcp_session_t;



#endif