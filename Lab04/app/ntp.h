#ifndef NTP_H
#define NTP_H

#include <stdio.h>
#include <arpa/inet.h>

#include "udp.h"
#include "tmp.h"

void ntp_sync();

#endif // NTP_H