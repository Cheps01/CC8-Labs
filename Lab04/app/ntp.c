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

typedef struct {
    const char *name;
    const char *ip;
    uint64_t offset;
    uint64_t delay;
    int valid;  // 0 if this server timed out
} server_result_t;

// NTP server list. Queries are made to the IP directly.
server_result_t servers[] = {
    {"time.google.com", "216.239.35.4", 0, 0, 0},
    {"time.apple.com", "17.253.6.45", 0, 0, 0},
    {"time.windows.com", "168.61.215.74", 0, 0, 0},
    {"time.cloudflare.com", "162.159.200.1", 0, 0, 0},
    {"time.nist.gov", "129.6.15.25", 0, 0, 0},
    {"time-a.nist.gov", "129.6.15.28", 0, 0, 0},
    {"pool.ntp.org", "23.186.168.124", 0, 0, 0},
};
int num_servers = sizeof(servers) / sizeof(servers[0]);

timestamp_t get_timestamp() {
    timestamp_t time;
    // Get seconds and milliseconds and convert to NTP format
    get_current_time();
    uint32_t timestamp_unix = get_seconds();
    uint32_t timestamp_ntp = timestamp_unix + (uint32_t)UNIX_TO_NTP_OFFSET;
    uint32_t milliseconds = get_miliseconds();
    uint32_t milliseconds_scaled = (uint32_t)((milliseconds / 1e3) * (2ULL << 31)); // scale = 2^32
    printf("seconds: %d milliseconds: %d\n", timestamp_unix, milliseconds_scaled);
    // Build client time struct
    time.sec = timestamp_ntp;
    time.frac = milliseconds_scaled;
    return time;
}

uint64_t compute_delay(timestamp_t t1, timestamp_t t2, timestamp_t t3, timestamp_t t4) {
    uint64_t t1_ms = ((uint64_t)t1.sec * 1000) + ((uint64_t)t1.frac * 1000 / (2ULL << 31));
    uint64_t t2_ms = ((uint64_t)t2.sec * 1000) + ((uint64_t)t2.frac * 1000 / (2ULL << 31));
    uint64_t t3_ms = ((uint64_t)t3.sec * 1000) + ((uint64_t)t3.frac * 1000 / (2ULL << 31));
    uint64_t t4_ms = ((uint64_t)t4.sec * 1000) + ((uint64_t)t4.frac * 1000 / (2ULL << 31));
    return (t4_ms - t1_ms) - (t3_ms - t2_ms);
}
uint64_t compute_offset(timestamp_t t1, timestamp_t t2, timestamp_t t3, timestamp_t t4) {
    uint64_t t1_ms = ((uint64_t)t1.sec * 1000) + ((uint64_t)t1.frac * 1000 / (2ULL << 31));
    uint64_t t2_ms = ((uint64_t)t2.sec * 1000) + ((uint64_t)t2.frac * 1000 / (2ULL << 31));
    uint64_t t3_ms = ((uint64_t)t3.sec * 1000) + ((uint64_t)t3.frac * 1000 / (2ULL << 31));
    uint64_t t4_ms = ((uint64_t)t4.sec * 1000) + ((uint64_t)t4.frac * 1000 / (2ULL << 31));
    return ((t2_ms - t1_ms) + (t3_ms - t4_ms)) / 2;
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
    printf("NTP packet built. Ready to send %d bytes.\n", (int)sizeof(packet));
    return packet;
}

ntp_packet_t parse_ntp_response(const void *buffer, size_t len) {
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

    return reply;
}

// Query a single NTP server for sync data
server_result_t query_server(const char* name, const char* ip) {
    server_result_t result;
    result.name = name;
    result.ip = ip;

    // Start socket early (always por 123 for NTP)
    start_udp_socket(ip, 123);

    // Get time and build NTP packet
    timestamp_t t1 = get_timestamp();
    ntp_packet_t request = build_ntp_packet(t1.sec, t1.frac);

    // Send UDP to defined server and port
    send_udp(&request, sizeof(request));

    // Read server response
    char resp[1024];
    ssize_t received = receive_udp(resp, sizeof(resp));

    // Record time as soon as UDP listener returns
    timestamp_t t4 = get_timestamp();

    // Check for incorrect response
    if (received < (ssize_t)sizeof(ntp_packet_t)) {
        result.valid = 0;
        return result;
    }
    print_hex((const unsigned char*)resp, received);

    // Parse server response and get timestamps
    ntp_packet_t response = parse_ntp_response(resp, received);
    timestamp_t t2 = {response.recv_ts_sec, response.recv_ts_frac};
    timestamp_t t3 = {response.tx_ts_sec, response.tx_ts_frac};

    // Calculate delay and offset
    uint64_t delay = compute_delay(t1, t2, t3, t4);
    result.delay = delay;
    uint64_t offset = compute_offset(t1, t2, t3, t4);
    result.offset = offset;
    result.valid = 1;

    return result;
}
// -----------------------------
//      USE CASE FUNCTIONS
// -----------------------------

// Sync algorithm
void query_all_servers() {
    server_result_t *best;

    // Iterate thorugh all servers
    for (int i = 0; i < num_servers; i++) {
        printf("-----------------------------\n");
        printf("Querying %s\n", servers[i].name);
        server_result_t res = query_server(servers[i].name, servers[i].ip);
        servers[i] = res;

        if (res.valid) {
            printf("%s offset=%ld delay=%ld\n", res.name, res.offset, res.delay);
            if (best == NULL || res.delay < best->delay) {
                best = &servers[i];
            }
        } else {
            printf("%s no response\n", res.name);
        }
        sleep(1);
    }

    // Apply best servers' offset
    if (best) {
        printf("Selected server: %s (delay=%ld)\n", best->name, best->delay);
        apply_time_offset(best->offset);
    } else {
        fprintf(stderr, "No servers responded\n");
    }
}
// Set the time to user specification
void time_set(int year, int month, int day, int hour, int minute, int second) {
    struct tm user_time = {0};

    user_time.tm_year = year - 1900;
    user_time.tm_mon  = month - 1;
    user_time.tm_mday = day;
    user_time.tm_hour = hour;
    user_time.tm_min = minute;
    user_time.tm_sec = second;

    set_internal_time(&user_time);
    printf("Internal clock set to %s", asctime(&user_time));
}