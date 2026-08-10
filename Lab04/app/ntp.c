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
    uint32_t delay = (t4 - t1) - (t3 - t2);
    printf("Delay: %u\n", delay);
    return delay;
}
uint32_t compute_offset(uint32_t t1, uint32_t t2, uint32_t t3, uint32_t t4) {
    uint32_t offset = ((t2 - t1) + (t3 - t4)) / 2;
    printf("Offset: %u\n", offset);
    return offset;
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

ntp_packet_t parse_ntp_response(const void *buffer, size_t len) {
    // Check for incorrect length of response
    if (len < sizeof(ntp_packet_t)) {
        fprintf(stderr, "Error: response too short (%zu bytes, expected %zu)\n", len, sizeof(ntp_packet_t));
        exit(EXIT_FAILURE);
    }
    // Copy data from buffer
    ntp_packet_t reply;
    memcpy(&reply, buffer, sizeof(ntp_packet_t));
    // Convert every multi-byte field from network to host byte order
    reply.root_delay      = ntohl(reply.root_delay);
    reply.root_dispersion = ntohl(reply.root_dispersion);
    reply.ref_id          = ntohl(reply.ref_id);
    reply.ref_ts_sec      = ntohl(reply.ref_ts_sec);
    reply.ref_ts_frac     = ntohl(reply.ref_ts_frac);
    reply.orig_ts_sec     = ntohl(reply.orig_ts_sec);
    reply.orig_ts_frac    = ntohl(reply.orig_ts_frac);
    reply.recv_ts_sec     = ntohl(reply.recv_ts_sec);   // T2
    reply.recv_ts_frac    = ntohl(reply.recv_ts_frac);
    reply.tx_ts_sec       = ntohl(reply.tx_ts_sec);     // T3
    reply.tx_ts_frac      = ntohl(reply.tx_ts_frac);
    printf("Server response packet rebuilt succesfully.\n");
    return reply;
}

// Run NTP sync algorithm
void ntp_sync() {
    // Start socket early
    start_udp_socket("132.163.96.6", 123);

    // Get time and build NTP packet
    timestamp_t t1 = get_timestamp();
    ntp_packet_t request = build_ntp_packet(t1.sec, t1.frac);

    // Send UDP to defined server and port
    send_udp(&request, sizeof(request));
    char resp[1024];
    ssize_t received = receive_udp(resp, sizeof(resp));
    print_hex((const unsigned char*)resp, received);

    // Parse server response and get timestamps
    ntp_packet_t response = parse_ntp_response(resp, received);
    timestamp_t t2 = {response.recv_ts_sec, response.recv_ts_frac};
    timestamp_t t3 = {response.tx_ts_sec, response.tx_ts_frac};

    // Record time as soon as UDP listener returns
    timestamp_t t4 = get_timestamp();

    // Calculate delay and offset
    uint32_t delay = compute_delay(t1.sec, t2.sec, t3.sec, t4.sec);
    uint32_t offset = compute_offset(t1.sec, t2.sec, t3.sec, t4.sec);
}