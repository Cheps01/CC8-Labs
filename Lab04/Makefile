CC = gcc
CFLAGS = -Wall -Wextra -Inetwork -Itime -Iapp
TARGET = ntp_client
OBJS = bin/main.o bin/udp.o bin/ntp.o bin/tmp.o bin/log.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

bin/main.o: main.c app/ntp.h app/log.h
	$(CC) $(CFLAGS) -c main.c -o bin/main.o

bin/ntp.o: app/ntp.c app/ntp.h network/udp.h time/tmp.h app/log.h
	$(CC) $(CFLAGS) -c app/ntp.c -o bin/ntp.o

bin/udp.o: network/udp.c network/udp.h
	$(CC) $(CFLAGS) -c network/udp.c -o bin/udp.o

bin/tmp.o: time/tmp.c time/tmp.h
	$(CC) $(CFLAGS) -c time/tmp.c -o bin/tmp.o

bin/log.o: app/log.c app/log.h
	$(CC) $(CFLAGS) -c app/log.c -o bin/log.o

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean