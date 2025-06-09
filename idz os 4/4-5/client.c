#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_MSG_LEN 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port> <proposal_text>\n", argv[0]);
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
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Send proposal to server
    sendto(sockfd, proposal, strlen(proposal), 0,
           (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("Proposal sent: \"%s\"\n", proposal);

    // Wait for server response
    ssize_t len = recvfrom(sockfd, buffer, MAX_MSG_LEN - 1, 0,
                           (struct sockaddr *)&server_addr, &addr_len);
    if (len < 0) {
        perror("recvfrom failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    buffer[len] = '\0';

    // Display reaction
    if (strcmp(buffer, "Согласна") == 0) {
        printf("Woohoo! She said yes!\n");
    } else {
        printf("Oh no... Maybe next year.\n");
    }

    close(sockfd);
    return 0;
}
