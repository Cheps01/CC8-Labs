#include "tmp.h"

struct timespec ts;
static double sim_offset_sec = 0.0;

// Let the user set an arbitrary starting date/time.
void set_internal_time(struct tm *user_datetime) {
    time_t user_epoch = timegm(user_datetime);  // interpret input as UTC
    // Compute difference between set real current time
    struct timespec real_ts;
    clock_gettime(CLOCK_REALTIME, &real_ts);
    double real_now = real_ts.tv_sec + real_ts.tv_nsec / 1e9;
    sim_offset_sec = (double)user_epoch - real_now;
}
// Apply offset to current internal time
void apply_time_offset(double correction_sec) {
    sim_offset_sec += correction_sec;
}
// Obtain time measures, adjusted by the simulated offset if one is set
void get_current_time() {
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        perror("Error: failed to get Unix timestamp");
        return;
    }

    if (sim_offset_sec != 0.0) {
        double adjusted = ts.tv_sec + ts.tv_nsec / 1e9 + sim_offset_sec;
        ts.tv_sec  = (time_t)adjusted;
        ts.tv_nsec = (long)((adjusted - ts.tv_sec) * 1e9);
    }
}
// Get seconds from timescec struct
uint32_t get_seconds() {
    return (uint32_t)ts.tv_sec;
}
// Get miliseconds from timespec struct
uint32_t get_miliseconds() {
    return (uint32_t)(ts.tv_nsec / 1e6);
}
// Get text timestamp
void get_datetime(char *buffer, size_t size) {
    struct tm *tm_info = localtime(&ts.tv_sec);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}