CC = gcc
CFLAGS = -Wall -Wextra -Inetwork -Idomain -Iapp
TARGET = ntp_client
OBJS = bin/main.o bin/udp.o bin/ntp.o bin/sync.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

bin/main.o: main.c app/sync.h
	$(CC) $(CFLAGS) -c main.c -o bin/main.o

bin/sync.o: app/sync.c app/sync.h network/udp.h domain/ntp.h
	$(CC) $(CFLAGS) -c app/sync.c -o bin/sync.o

bin/udp.o: network/udp.c network/udp.h
	$(CC) $(CFLAGS) -c network/udp.c -o bin/udp.o

bin/ntp.o: domain/ntp.c domain/ntp.h
	$(CC) $(CFLAGS) -c domain/ntp.c -o bin/ntp.o

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean