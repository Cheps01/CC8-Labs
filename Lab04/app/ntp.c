#include "ntp.h"

#define UNIX_TO_NTP_OFFSET  2208988800; // seconds from 1900 to 1970

void ntp_init() {
    // Get current timestamps from the local system
    get_current_time();
    uint32_t timestamp_unix = get_unix_timestamp();
    printf("Unix timestamp (seconds): %u\n", timestamp_unix);
    uint32_t timestamp_ntp = timestamp_unix + (uint32_t)UNIX_TO_NTP_OFFSET;
    printf("NTP timestamp (seconds): %u\n", timestamp_ntp);
    uint32_t milliseconds = get_miliseconds();
    printf("Miliseconds: %u\n", milliseconds);
    uint32_t milliseconds_scaled = (uint32_t)((milliseconds / 1e3) * (2ULL << 31)); // scale = 2^32
    printf("Miliseconds (scaled): %u\n", milliseconds_scaled);
}