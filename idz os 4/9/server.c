#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>
#include <errno.h>

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

Logger loggers[MAX_LOGGERS];
int num_loggers = 0;

void add_logger(const char* ip, int port) {
    // Проверка на максимальное количество логгеров
    if (num_loggers >= MAX_LOGGERS) {
        fprintf(stderr, "Cannot add more loggers. Maximum %d reached.\n", MAX_LOGGERS);
        return;
    }

    // Проверка на дубликаты
    for (int i = 0; i < num_loggers; i++) {
        if (loggers[i].addr.sin_port == htons(port) && 
            strcmp(inet_ntoa(loggers[i].addr.sin_addr), ip) == 0) {
            loggers[i].active = 1;
            loggers[i].last_active = time(NULL);
            printf("Logger reactivated: %s:%d\n", ip, port);
            return;
        }
    }

    // Добавление нового логгера
    memset(&loggers[num_loggers].addr, 0, sizeof(loggers[num_loggers].addr));
    loggers[num_loggers].addr.sin_family = AF_INET;
    loggers[num_loggers].addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &loggers[num_loggers].addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        return;
    }

    loggers[num_loggers].active = 1;
    loggers[num_loggers].last_active = time(NULL);
    printf("New logger registered: %s:%d (%d/%d)\n", 
           ip, port, num_loggers + 1, MAX_LOGGERS);
    num_loggers++;
}

void remove_logger(const char* ip, int port) {
    for (int i = 0; i < num_loggers; i++) {
        if (loggers[i].addr.sin_port == htons(port) && 
            strcmp(inet_ntoa(loggers[i].addr.sin_addr), ip) == 0) {
            loggers[i].active = 0;
            printf("Logger unregistered: %s:%d\n", ip, port);
            return;
        }
    }
    printf("Logger not found: %s:%d\n", ip, port);
}

void send_to_loggers(int logfd, const char* msg) {
    for (int i = 0; i < num_loggers; i++) {
        if (loggers[i].active) {
            if (sendto(logfd, msg, strlen(msg), 0,
                     (struct sockaddr*)&loggers[i].addr, 
                     sizeof(loggers[i].addr)) < 0) {
                perror("Failed to send to logger");
                loggers[i].active = 0;
            }
        }
    }
}

void check_heartbeats(int logfd) {
    static time_t last_check = 0;
    time_t now = time(NULL);

    if (now - last_check >= HEARTBEAT_INTERVAL) {
        for (int i = 0; i < num_loggers; i++) {
            if (loggers[i].active) {
                // Проверка таймаута
                if (now - loggers[i].last_active >= LOGGER_TIMEOUT) {
                    printf("Logger timeout: %s:%d\n",
                          inet_ntoa(loggers[i].addr.sin_addr),
                          ntohs(loggers[i].addr.sin_port));
                    loggers[i].active = 0;
                } else {
                    // Отправка ping
                    if (sendto(logfd, "PING", 4, 0,
                             (struct sockaddr*)&loggers[i].addr,
                             sizeof(loggers[i].addr)) < 0) {
                        perror("Failed to send PING");
                        loggers[i].active = 0;
                    }
                }
            }
        }
        last_check = now;
    }
}

void handle_logger_command(int logfd, const char* cmd, 
                         const struct sockaddr_in* logger_addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &logger_addr->sin_addr, ip, INET_ADDRSTRLEN);
    int port = ntohs(logger_addr->sin_port);

    if (strncmp(cmd, "REGISTER", 8) == 0) {
        add_logger(ip, port);
    } 
    else if (strncmp(cmd, "UNREGISTER", 10) == 0) {
        remove_logger(ip, port);
    }
    else if (strncmp(cmd, "PONG", 4) == 0) {
        for (int i = 0; i < num_loggers; i++) {
            if (memcmp(&loggers[i].addr.sin_addr, &logger_addr->sin_addr, 
                      sizeof(logger_addr->sin_addr)) == 0 &&
                loggers[i].addr.sin_port == logger_addr->sin_port) {
                loggers[i].last_active = time(NULL);
                break;
            }
        }
    }
    else if (strncmp(cmd, "STATUS", 6) == 0) {
        int active_count = 0;
        for (int i = 0; i < num_loggers; i++) {
            if (loggers[i].active) active_count++;
        }
        
        char status[256];
        snprintf(status, sizeof(status), 
                "STATUS: %d active loggers (%d total)",
                active_count, num_loggers);
        
        sendto(logfd, status, strlen(status), 0,
              (struct sockaddr*)logger_addr, sizeof(*logger_addr));
    }
    else {
        printf("Unknown command from %s:%d: %s\n", ip, port, cmd);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <port> <num_clients> [<logger_ip> <logger_port> ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int num_clients = atoi(argv[2]);

    // Инициализация сокетов
    int sockfd, logfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ||
        (logfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        close(logfd);
        exit(EXIT_FAILURE);
    }

    // Добавление логгеров из аргументов командной строки
    for (int i = 3; i + 1 < argc; i += 2) {
        add_logger(argv[i], atoi(argv[i + 1]));
    }

    printf("Server started on port %d\n", port);
    printf("Waiting for %d client proposals...\n", num_clients);

    ClientData clients[MAX_CLIENTS];
    int received = 0;
    fd_set readfds;
    int maxfd = (sockfd > logfd) ? sockfd : logfd;

    // Основной цикл обработки
    while (received < num_clients) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(logfd, &readfds);

        struct timeval timeout = {1, 0}; // 1 секунда таймаута
        int activity = select(maxfd + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0 && errno != EINTR) {
            perror("select error");
            break;
        }

        // Проверка активности логгеров
        check_heartbeats(logfd);

        if (activity > 0) {
            // Обработка команд от логгеров
            if (FD_ISSET(logfd, &readfds)) {
                struct sockaddr_in logger_addr;
                socklen_t addr_len = sizeof(logger_addr);
                char buffer[MAX_MSG_LEN];

                ssize_t len = recvfrom(logfd, buffer, sizeof(buffer) - 1, 0,
                                     (struct sockaddr*)&logger_addr, &addr_len);
                if (len > 0) {
                    buffer[len] = '\0';
                    handle_logger_command(logfd, buffer, &logger_addr);
                }
            }

            // Обработка предложений от клиентов
            if (FD_ISSET(sockfd, &readfds)) {
                struct sockaddr_in client_addr;
                socklen_t addr_len = sizeof(client_addr);
                char buffer[MAX_MSG_LEN];

                ssize_t len = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                      (struct sockaddr*)&client_addr, &addr_len);
                if (len > 0) {
                    buffer[len] = '\0';
                    
                    // Сохранение клиента
                    clients[received].addr = client_addr;
                    strncpy(clients[received].message, buffer, MAX_MSG_LEN);
                    received++;

                    // Логирование
                    if (num_loggers > 0) {
                        char log_msg[MAX_MSG_LEN + 100];
                        snprintf(log_msg, sizeof(log_msg),
                                "Proposal %d/%d from %s:%d: %s",
                                received, num_clients,
                                inet_ntoa(client_addr.sin_addr),
                                ntohs(client_addr.sin_port),
                                buffer);
                        send_to_loggers(logfd, log_msg);
                    }
                }
            }
        }
    }

    // Выбор лучшего предложения
    int best = 0;
    for (int i = 1; i < num_clients; i++) {
        if (strlen(clients[i].message) > strlen(clients[best].message)) {
            best = i;
        }
    }

    // Логирование результата
    if (num_loggers > 0) {
        char result_msg[MAX_MSG_LEN + 100];
        snprintf(result_msg, sizeof(result_msg),
                "Selected best proposal: %s (from %s:%d)",
                clients[best].message,
                inet_ntoa(clients[best].addr.sin_addr),
                ntohs(clients[best].addr.sin_port));
        send_to_loggers(logfd, result_msg);
    }

    // Отправка ответов клиентам
    for (int i = 0; i < num_clients; i++) {
        const char* response = (i == best) ? "AGREE" : "REJECT";
        
        if (sendto(sockfd, response, strlen(response), 0,
                  (struct sockaddr*)&clients[i].addr,
                  sizeof(clients[i].addr)) < 0) {
            perror("Failed to send response");
        }

        // Логирование ответа
        if (num_loggers > 0) {
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg),
                    "Sent %s to %s:%d",
                    response,
                    inet_ntoa(clients[i].addr.sin_addr),
                    ntohs(clients[i].addr.sin_port));
            send_to_loggers(logfd, log_msg);
        }
    }

    printf("Server completed. Best proposal selected.\n");
    close(sockfd);
    close(logfd);
    return 0;
}