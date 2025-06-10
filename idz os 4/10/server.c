#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>

#define MAX_CLIENTS 100
#define MAX_MSG_LEN 1024
#define MAX_LOGGERS 10
#define HEARTBEAT_INTERVAL 30
#define LOGGER_TIMEOUT (HEARTBEAT_INTERVAL * 2)

typedef struct {
    struct sockaddr_in addr;
    int active;
    time_t last_active;
} Logger;

typedef struct {
    struct sockaddr_in addr;
    char message[MAX_MSG_LEN];
} ClientData;

volatile sig_atomic_t shutdown_requested = false;
Logger loggers[MAX_LOGGERS];
int num_loggers = 0;
ClientData clients[MAX_CLIENTS];
int received_clients = 0;

void handle_sigint(int sig) {
    shutdown_requested = true;
}

void add_logger(const char* ip, int port) {
    if (num_loggers >= MAX_LOGGERS) return;

    for (int i = 0; i < num_loggers; i++) {
        if (loggers[i].addr.sin_port == htons(port) && 
            strcmp(inet_ntoa(loggers[i].addr.sin_addr), ip) == 0) {
            loggers[i].active = 1;
            loggers[i].last_active = time(NULL);
            return;
        }
    }

    memset(&loggers[num_loggers].addr, 0, sizeof(loggers[num_loggers].addr));
    loggers[num_loggers].addr.sin_family = AF_INET;
    loggers[num_loggers].addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &loggers[num_loggers].addr.sin_addr);
    loggers[num_loggers].active = 1;
    loggers[num_loggers].last_active = time(NULL);
    num_loggers++;
}

void send_to_loggers(int logfd, const char* msg) {
    for (int i = 0; i < num_loggers; i++) {
        if (loggers[i].active) {
            sendto(logfd, msg, strlen(msg), 0,
                  (struct sockaddr*)&loggers[i].addr, sizeof(loggers[i].addr));
        }
    }
}

void notify_shutdown(int sockfd, int logfd) {
    // Notify all connected clients
    for (int i = 0; i < received_clients; i++) {
        sendto(sockfd, "SHUTDOWN", 8, 0,
              (struct sockaddr*)&clients[i].addr, sizeof(clients[i].addr));
    }
    
    // Notify loggers
    if (num_loggers > 0) {
        send_to_loggers(logfd, "SERVER SHUTTING DOWN");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <port> <num_clients> [<logger_ip> <logger_port>...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Setup signal handler
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    int port = atoi(argv[1]);
    int num_clients = atoi(argv[2]);
    int sockfd, logfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ||
        (logfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Add loggers from command line
    for (int i = 3; i + 1 < argc; i += 2) {
        add_logger(argv[i], atoi(argv[i + 1]));
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        close(logfd);
        exit(EXIT_FAILURE);
    }

    fd_set readfds;
    int maxfd = (sockfd > logfd) ? sockfd : logfd;

    while (!shutdown_requested && received_clients < num_clients) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(logfd, &readfds);

        struct timeval tv = {1, 0};
        int activity = select(maxfd + 1, &readfds, NULL, NULL, &tv);

        if (activity < 0 && errno != EINTR) {
            perror("select");
            break;
        }

        if (activity > 0) {
            if (FD_ISSET(sockfd, &readfds)) {
                struct sockaddr_in client_addr;
                socklen_t addr_len = sizeof(client_addr);
                char buffer[MAX_MSG_LEN];

                ssize_t len = recvfrom(sockfd, buffer, MAX_MSG_LEN - 1, 0,
                                      (struct sockaddr*)&client_addr, &addr_len);
                if (len > 0) {
                    buffer[len] = '\0';
                    clients[received_clients].addr = client_addr;
                    strncpy(clients[received_clients].message, buffer, MAX_MSG_LEN);
                    received_clients++;

                    if (num_loggers > 0) {
                        char log_msg[MAX_MSG_LEN + 100];
                        snprintf(log_msg, sizeof(log_msg), 
                                "Client %d/%d: %s:%d - %s",
                                received_clients, num_clients,
                                inet_ntoa(client_addr.sin_addr),
                                ntohs(client_addr.sin_port),
                                buffer);
                        send_to_loggers(logfd, log_msg);
                    }
                }
            }

            if (FD_ISSET(logfd, &readfds)) {
                struct sockaddr_in logger_addr;
                socklen_t addr_len = sizeof(logger_addr);
                char buffer[MAX_MSG_LEN];

                ssize_t len = recvfrom(logfd, buffer, MAX_MSG_LEN - 1, 0,
                                     (struct sockaddr*)&logger_addr, &addr_len);
                if (len > 0) {
                    buffer[len] = '\0';
                    if (strncmp(buffer, "REGISTER", 8) == 0) {
                        char ip[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &logger_addr.sin_addr, ip, INET_ADDRSTRLEN);
                        add_logger(ip, ntohs(logger_addr.sin_port));
                    }
                }
            }
        }
    }

    if (shutdown_requested) {
        notify_shutdown(sockfd, logfd);
    } else {
        // Normal processing - select best proposal
        int best = 0;
        for (int i = 1; i < num_clients; i++) {
            if (strlen(clients[i].message) > strlen(clients[best].message)) {
                best = i;
            }
        }

        // Send responses
        for (int i = 0; i < num_clients; i++) {
            const char* response = (i == best) ? "AGREE" : "REJECT";
            sendto(sockfd, response, strlen(response), 0,
                  (struct sockaddr*)&clients[i].addr, sizeof(clients[i].addr));
        }
    }

    close(sockfd);
    close(logfd);
    return 0;
}