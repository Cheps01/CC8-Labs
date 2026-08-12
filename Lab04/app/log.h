#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>

void log_init();
void log_close();
void log_msg(const char *fmt, ...);

#endif // LOG_H