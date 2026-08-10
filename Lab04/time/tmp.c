#include "tmp.h"

struct timespec ts;

// Obtain time measures from January 1st, 1970 00:00 UTC
void get_current_time() {
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        perror("Error: failed to get Unix timestamp");
    }
}
// Get seconds from timescec struct
uint32_t get_unix_timestamp() {
    return (uint32_t)ts.tv_sec;
}
// Get miliseconds from timespec struct
uint32_t get_miliseconds() {
    return (uint32_t)(ts.tv_nsec / 1e6);
}