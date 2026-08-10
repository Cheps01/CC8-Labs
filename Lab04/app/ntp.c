#include "ntp.h"

#define UNIX_TO_NTP_OFFSET  2208988800  // seconds from 1900 to 1970

#define LI                  0           // no leaps
#define VN                  4           // version 3
#define MODE                3           // client mode

#pragma pack(push, 1)
typedef struct {
    // Common fields
    uint8_t  li_vn_mode;                // Leap Indicator (2 bits) + Version (3 bits) + Mode (3 bits)
    uint8_t  stratum;                   
    uint8_t  poll;                      
    int8_t   precision;                 

    // Time fields
    uint32_t root_delay;                
    uint32_t root_dispersion;           
    uint32_t ref_id;                    

    uint32_t ref_ts_sec;                
    uint32_t ref_ts_frac;               

    uint32_t orig_ts_sec;               // origin timestamp, seconds   (T1, filled in by server on reply)
    uint32_t orig_ts_frac;              // origin timestamp, fraction

    uint32_t recv_ts_sec;               
    uint32_t recv_ts_frac;              

    uint32_t tx_ts_sec;                 // transmit timestamp, seconds (T1 on request, T3 on reply)
    uint32_t tx_ts_frac;                // transmit timestamp, fraction
} ntp_packet_t;
#pragma pack(pop)

typedef struct {
    uint32_t sec;
    uint32_t frac;
} timestamp_t;

timestamp_t get_timestamp() {
    timestamp_t time;
    // Get seconds and milliseconds and convert to NTP format
    get_current_time();
    uint32_t timestamp_unix = get_unix_timestamp();
    printf("Unix timestamp (seconds): %u\n", timestamp_unix);
    uint32_t timestamp_ntp = timestamp_unix + (uint32_t)UNIX_TO_NTP_OFFSET;
    printf("NTP timestamp (seconds): %u\n", timestamp_ntp);
    uint32_t milliseconds = get_miliseconds();
    printf("Miliseconds: %u\n", milliseconds);
    uint32_t milliseconds_scaled = (uint32_t)((milliseconds / 1e3) * (2ULL << 31)); // scale = 2^32
    printf("Miliseconds (scaled): %u\n", milliseconds_scaled);
    // Build client time struct
    time.sec = timestamp_ntp;
    time.frac = milliseconds_scaled;
    return time;
}
uint32_t compute_delay(uint32_t t1, uint32_t t2, uint32_t t3, uint32_t t4) {
    return (t4 - t1) - (t3 - t2);
}
uint32_t compute_offset(uint32_t t1, uint32_t t2, uint32_t t3, uint32_t t4) {
    return ((t2 - t1) + (t3 - t4)) / 2;
}
ntp_packet_t build_ntp_packet(uint32_t t1_sec, uint32_t t1_frac) {
    ntp_packet_t packet;
    // set everything to zero
    memset(&packet, 0, sizeof(packet));
    // Build only required fields
    packet.li_vn_mode = (LI << 6) | (VN << 3) | MODE;
    packet.orig_ts_sec = htonl(t1_sec);
    packet.orig_ts_frac = htonl(t1_frac);
    packet.tx_ts_sec = htonl(t1_sec);
    packet.tx_ts_frac = htonl(t1_frac); 
    // Sanity check
    printf("NTP packet built. Ready to send %d bytes\n", (int)sizeof(packet));
    return packet;
}
void print_hex(const unsigned char *buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x ", buf[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}
// Run NTP sync algorithm
void ntp_sync() {
    // Start socket early to minimize offset and delay
    start_udp_socket("132.163.96.6", 123);

    // Get time and build NTP packet
    timestamp_t t1 = get_timestamp();
    ntp_packet_t ntpacket = build_ntp_packet(t1.sec, t1.frac);

    // Send UDP to defined server and port
    send_udp(&ntpacket, sizeof(ntpacket));
    char* resp = receive_udp();
    print_hex((const unsigned char*)resp, sizeof(resp));

    // Record time as soon as UDP listener returns
    timestamp_t t4 = get_timestamp();
}