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
    if (argc != 3 && argc != 5) {
        fprintf(stderr, "Usage: %s <port> <num_clients> [<logger_ip> <logger_port>]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int num_clients = atoi(argv[2]);

    int sockfd, logfd = -1;
    struct sockaddr_in server_addr, client_addr, logger_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[MAX_MSG_LEN];
    ClientData clients[MAX_CLIENTS];
    int received = 0;
    int use_logger = 0;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (argc == 5) {
        use_logger = 1;
        if ((logfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("logger socket");
            exit(EXIT_FAILURE);
        }

        memset(&logger_addr, 0, sizeof(logger_addr));
        logger_addr.sin_family = AF_INET;
        logger_addr.sin_port = htons(atoi(argv[4]));
        if (inet_pton(AF_INET, argv[3], &logger_addr.sin_addr) <= 0) {
            perror("logger inet_pton");
            exit(EXIT_FAILURE);
        }
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Receive proposals
    while (received < num_clients) {
        ssize_t len = recvfrom(sockfd, buffer, MAX_MSG_LEN - 1, 0,
                               (struct sockaddr *)&client_addr, &addr_len);
        if (len < 0) continue;

        buffer[len] = '\0';

        clients[received].addr = client_addr;
        strncpy(clients[received].message, buffer, MAX_MSG_LEN);

        if (use_logger) {
            char log_msg[MAX_MSG_LEN + 100];
            snprintf(log_msg, sizeof(log_msg), "Proposal received from %s:%d: %s",
                     inet_ntoa(client_addr.sin_addr),
                     ntohs(client_addr.sin_port),
                     buffer);
            sendto(logfd, log_msg, strlen(log_msg), 0,
                   (struct sockaddr *)&logger_addr, sizeof(logger_addr));
        }

        received++;
    }

    // Choose best proposal
    int best = 0;
    for (int i = 1; i < num_clients; ++i) {
        if (strlen(clients[i].message) > strlen(clients[best].message)) {
            best = i;
        }
    }

    if (use_logger) {
        char result[MAX_MSG_LEN + 100];
        snprintf(result, sizeof(result), "Best proposal chosen: %s", clients[best].message);
        sendto(logfd, result, strlen(result), 0,
               (struct sockaddr *)&logger_addr, sizeof(logger_addr));
    }

    // Send responses
    for (int i = 0; i < num_clients; ++i) {
        const char *response = (i == best) ? "Agree" : "Reject";
        sendto(sockfd, response, strlen(response), 0,
               (struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr));

        if (use_logger) {
            char msg[256];
            snprintf(msg, sizeof(msg),
                     "Response sent to %s:%d: %s",
                     inet_ntoa(clients[i].addr.sin_addr),
                     ntohs(clients[i].addr.sin_port),
                     response);
            sendto(logfd, msg, strlen(msg), 0,
                   (struct sockaddr *)&logger_addr, sizeof(logger_addr));
        }
    }

    close(sockfd);
    if (use_logger) close(logfd);
    return 0;
}
