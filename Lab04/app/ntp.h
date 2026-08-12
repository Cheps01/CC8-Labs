#ifndef NTP_H
#define NTP_H

#include <stdio.h>
#include <arpa/inet.h>

#include "udp.h"
#include "tmp.h"
#include "log.h"

void ntp_sync();
void time_set(int year, int month, int day, int hour, int minute, int second);

#endif // NTP_H