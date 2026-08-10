#include <stdio.h>
#include <string.h>

#include "ntp.h"

int main(int argc, char *argv[]) {
    printf("Running NTP client\n");
    // Parse arguments
    for (int i=1; i<argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'd':
                    printf("Dates is %s\n", argv[i + 1]);
                    break;
                case 't':
                    printf("Time is %s\n", argv[i + 1]);
                    break;
                default:
                    printf("Unrecognized argument: %s\n", argv[i]);
            }
        }
    }
    ntp_init();
    return 0;
}