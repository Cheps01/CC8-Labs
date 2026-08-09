#include "udp.h"

#define BUFFER_SIZE      1024

char buffer[BUFFER_SIZE];
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
    printf("Initialized socket correctly\n");
}
void send_udp(const char *message) {
    // Send UDP datagrams
    ssize_t sent = sendto(
        sockfd, 
        message, 
        strlen(message), 
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
void receive_udp() {
    // Receive UDP datagrams
    socklen_t from_len = sizeof(from_addr);
    ssize_t received = recvfrom(
        sockfd, 
        buffer, 
        sizeof(buffer) - 1, 
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
    buffer[received] = '\0';
    printf("Received %zd bytes: %s\n", received, buffer);
    // Close UDP socket
    close(sockfd);
}