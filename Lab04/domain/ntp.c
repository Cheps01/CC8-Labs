#include "ntp.h"

int64_t get_time() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ((int64_t)ts.tv_sec) * 1000 + ((int64_t)ts.tv_nsec) / 1000000;
}