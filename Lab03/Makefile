CC = gcc

CFLAGS = -O2 -Wall -Wextra

SRC = lab_3.c
OUT = udp_test

JAVA_SRC = UdpServer.java DnsHandler.java DnsQuery.java
JAVAC = javac

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC)

run: $(OUT)
	./$(OUT)

javac: $(JAVA_SRC)
	$(JAVAC) $(JAVA_SRC)

runjava: javac
	java UdpServer port 53

runbroadcast:
	javac UdpBroadcastSender.java && java UdpBroadcastSender

killudp:
	sudo kill -9 $(sudo lsof -t -iUDP:9999)

clean:
	rm -f $(OUT) *.class