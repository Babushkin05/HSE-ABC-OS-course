#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_MSG_LEN 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port> <proposal>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    const char *proposal = argv[3];

    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[MAX_MSG_LEN];
    socklen_t addr_len = sizeof(server_addr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    sendto(sockfd, proposal, strlen(proposal), 0,
          (struct sockaddr *)&server_addr, sizeof(server_addr));

    ssize_t len = recvfrom(sockfd, buffer, MAX_MSG_LEN - 1, 0,
                          (struct sockaddr *)&server_addr, &addr_len);
    if (len > 0) {
        buffer[len] = '\0';
        if (strcmp(buffer, "SHUTDOWN") == 0) {
            printf("Server is shutting down. Goodbye!\n");
        } 
    } else {
        perror("recvfrom");
    }

    close(sockfd);
    return 0;
}