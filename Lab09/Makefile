CC = gcc
CFLAGS = -Wall -Wextra -Inetwork -Iapp
TARGET = speed_test
OBJS = bin/main.o bin/speed_test.o bin/raw_socket.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

bin/main.o: main.c app/speed_test.h network/raw_socket.h
	$(CC) $(CFLAGS) -c main.c -o bin/main.o

bin/raw_socket.o: network/raw_socket.c network/raw_socket.h 
	$(CC) $(CFLAGS) -c network/raw_socket.c -o bin/raw_socket.o

bin/speed_test.o: app/speed_test.c app/speed_test.h
	$(CC) $(CFLAGS) -c app/speed_test.c -o bin/speed_test.o

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean