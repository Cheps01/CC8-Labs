#include "log.h"

static FILE *log_file = NULL;

void log_init() {
    mkdir("logs", 0755);
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char path[128];
    snprintf(path, sizeof(path), "logs/ntp_sync_%04d%02d%02d_%02d%02d%02d.log",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
    log_file = fopen(path, "w");
    if (!log_file) {
        perror("Error: failed to open log file");
        exit(EXIT_FAILURE);
    }
}

void log_close() {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}

void log_msg(const char *fmt, ...) {
    va_list args;

    // Display to console
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");

    // Write to log file
    if (log_file) {
        va_start(args, fmt);
        vfprintf(log_file, fmt, args);
        va_end(args);
        fprintf(log_file, "\n");
        fflush(log_file);  // flush immediately so a crash doesn't lose recent entries
    }
}