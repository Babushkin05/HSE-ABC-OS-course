#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define MAX_CLIENTS 100
#define MAX_MSG_LEN 1024
#define MAX_LOGGERS 10

typedef struct {
    struct sockaddr_in addr;
    int active;
} Logger;

typedef struct {
    struct sockaddr_in addr;
    char message[MAX_MSG_LEN];
} ClientData;

Logger loggers[MAX_LOGGERS];
int num_loggers = 0;

void add_logger(const char* ip, int port) {
    if (num_loggers >= MAX_LOGGERS) return;
    
    memset(&loggers[num_loggers].addr, 0, sizeof(loggers[num_loggers].addr));
    loggers[num_loggers].addr.sin_family = AF_INET;
    loggers[num_loggers].addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &loggers[num_loggers].addr.sin_addr);
    loggers[num_loggers].active = 1;
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

int main(int argc, char *argv[]) {
    if (argc < 3 || (argc - 3) % 2 != 0) {
        fprintf(stderr, "Usage: %s <port> <num_clients> [<logger_ip> <logger_port> ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int num_clients = atoi(argv[2]);

    int sockfd, logfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[MAX_MSG_LEN];
    ClientData clients[MAX_CLIENTS];
    int received = 0;

    // Инициализация сокетов
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ||
        (logfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Добавление логгеров
    for (int i = 3; i < argc; i += 2) {
        add_logger(argv[i], atoi(argv[i+1]));
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    fd_set readfds;
    struct timeval tv;

    // Получение предложений
    while (received < num_clients) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int activity = select(sockfd + 1, &readfds, NULL, NULL, &tv);
        
        if (activity > 0 && FD_ISSET(sockfd, &readfds)) {
            ssize_t len = recvfrom(sockfd, buffer, MAX_MSG_LEN - 1, 0,
                                  (struct sockaddr *)&client_addr, &addr_len);
            if (len < 0) continue;

            buffer[len] = '\0';
            clients[received].addr = client_addr;
            strncpy(clients[received].message, buffer, MAX_MSG_LEN);

            if (num_loggers > 0) {
                char log_msg[MAX_MSG_LEN + 100];
                snprintf(log_msg, sizeof(log_msg), "Proposal received from %s:%d: %s",
                         inet_ntoa(client_addr.sin_addr),
                         ntohs(client_addr.sin_port),
                         buffer);
                send_to_loggers(logfd, log_msg);
            }
            received++;
        }
    }

    // Выбор лучшего предложения
    int best = 0;
    for (int i = 1; i < num_clients; ++i) {
        if (strlen(clients[i].message) > strlen(clients[best].message)) {
            best = i;
        }
    }

    if (num_loggers > 0) {
        char result[MAX_MSG_LEN + 100];
        snprintf(result, sizeof(result), "Best proposal chosen: %s", clients[best].message);
        send_to_loggers(logfd, result);
    }

    // Отправка ответов
    for (int i = 0; i < num_clients; ++i) {
        const char *response = (i == best) ? "Agree" : "Reject";
        sendto(sockfd, response, strlen(response), 0,
              (struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr));

        if (num_loggers > 0) {
            char msg[256];
            snprintf(msg, sizeof(msg),
                    "Response sent to %s:%d: %s",
                    inet_ntoa(clients[i].addr.sin_addr),
                    ntohs(clients[i].addr.sin_port),
                    response);
            send_to_loggers(logfd, msg);
        }
    }

    close(sockfd);
    close(logfd);
    return 0;
}