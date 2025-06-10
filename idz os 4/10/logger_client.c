#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_MSG_LEN 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int sockfd;
    struct sockaddr_in addr;
    char buffer[MAX_MSG_LEN];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    printf("Logger started on port %d\n", port);

    while (1) {
        ssize_t len = recvfrom(sockfd, buffer, MAX_MSG_LEN - 1, 0, NULL, NULL);
        if (len > 0) {
            buffer[len] = '\0';
            printf("[LOG] %s\n", buffer);
        }
    }

    close(sockfd);
    return 0;
}