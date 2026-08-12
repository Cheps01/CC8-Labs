#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "ntp.h"

int main(int argc, char *argv[]) {
    log_init();
    log_msg("NTP client started.");

    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);

    int year   = tm_now->tm_year + 1900;
    int month  = tm_now->tm_mon + 1;
    int day    = tm_now->tm_mday;
    int hour   = tm_now->tm_hour;
    int minute = tm_now->tm_min;
    int second = tm_now->tm_sec;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (i + 1 >= argc) {
                log_msg("Error: option '%s' requires a value", argv[i]);
                log_close();
                return 1;
            }
            switch (argv[i][1]) {
                case 'Y': year   = atoi(argv[++i]); break;
                case 'M': month  = atoi(argv[++i]); break;
                case 'D': day    = atoi(argv[++i]); break;
                case 'h': hour   = atoi(argv[++i]); break;
                case 'm': minute = atoi(argv[++i]); break;
                case 's': second = atoi(argv[++i]); break;
                default:
                    log_msg("Unrecognized argument: %s", argv[i]);
                    log_close();
                    return 1;
            }
        }
    }

    log_msg("Setting initial time to %04d-%02d-%02d %02d:%02d:%02d",
            year, month, day, hour, minute, second);
    time_set(year, month, day, hour, minute, second);

    ntp_sync();
    log_close();
    return 0;
}
