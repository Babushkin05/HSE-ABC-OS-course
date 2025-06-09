#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 100
#define MAX_MSG_LEN 1024

typedef struct {
    struct sockaddr_in addr;
    char message[MAX_MSG_LEN];
} ClientData;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <num_clients>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int num_clients = atoi(argv[2]);

    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[MAX_MSG_LEN];
    socklen_t addr_len = sizeof(client_addr);

    ClientData clients[MAX_CLIENTS];
    int received = 0;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server started. Waiting for %d proposals...\n", num_clients);

    while (received < num_clients) {
        memset(buffer, 0, MAX_MSG_LEN);
        ssize_t len = recvfrom(sockfd, buffer, MAX_MSG_LEN - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (len < 0) {
            perror("recvfrom failed");
            continue;
        }

        buffer[len] = '\0';
        printf("Received proposal from %s:%d: %s\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port),
               buffer);

        clients[received].addr = client_addr;
        strncpy(clients[received].message, buffer, MAX_MSG_LEN);
        received++;
    }

    // выбирает самое лучшее, самое длинное)
    int best_index = 0;
    for (int i = 1; i < num_clients; ++i) {
        if (strlen(clients[i].message) > strlen(clients[best_index].message)) {
            best_index = i;
        }
    }

    printf("Selecting the best proposal...\n");

    // Send responses
    for (int i = 0; i < num_clients; ++i) {
        const char *response = (i == best_index) ? "Согласна" : "Отказ";
        sendto(sockfd, response, strlen(response), 0,
               (struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr));
    }

    printf("Chosen proposal: %s\n", clients[best_index].message);
    printf("Responses sent. Shutting down.\n");

    close(sockfd);
    return 0;
}
