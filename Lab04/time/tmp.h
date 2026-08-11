#ifndef TMP_H
#define TMP_H

#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void set_internal_time(struct tm *user_datetime);
void apply_time_offset(double correction_sec);
void get_current_time();
uint32_t get_seconds();
uint32_t get_miliseconds();
void get_datetime(char *buffer, size_t size);

#endif // TMP_H