#include "udp.h"

int sockfd;
struct sockaddr_in server_addr;
struct sockaddr_in from_addr;

void start_udp_socket(const char* server_ip, const int server_port) {
    // Initialize socket and get file descriptor
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("Error: failed to create socket");
        exit(EXIT_FAILURE);
    }
    // Build destination address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Error: failed to convert IP address");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Initialized UDP socket correctly\n");
}
void send_udp(const void *message, size_t len) {
    // Send UDP datagrams
    ssize_t sent = sendto(
        sockfd, 
        message, 
        len, 
        0, 
        (struct sockaddr *)&server_addr, 
        sizeof(server_addr)
    );
    // Check sending error
    if (sent < 0) {
        perror("Error: failed to send message");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Sent %zd bytes\n", sent);
}
ssize_t receive_udp(void *buffer, size_t len) {
    // Receive UDP datagrams
    socklen_t from_len = sizeof(from_addr);
    ssize_t received = recvfrom(
        sockfd, 
        buffer, 
        len, 
        0,
        (struct sockaddr *)&from_addr, 
        &from_len
    );
    // Check for receiving errors
    if (received < 0) {
        perror("Error: failed to receive message");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Received %zd bytes\n", received);
    return received;
}
void print_hex(const unsigned char *buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x ", buf[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
}