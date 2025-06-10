#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_MSG_LEN 1024

void send_command(const char* server_ip, int server_port, const char* cmd) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("command socket");
        return;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &addr.sin_addr);

    sendto(sock, cmd, strlen(cmd), 0, (struct sockaddr*)&addr, sizeof(addr));
    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc != 2 && argc != 4) {
        fprintf(stderr, "Usage: %s <port> [<server_ip> <server_port>]\n", argv[0]);
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

    printf("Logger client started on port %d\n", port);

    // Авторегистрация при указании сервера
    if (argc == 4) {
        const char* server_ip = argv[2];
        int server_port = atoi(argv[3]);
        char cmd[50];
        snprintf(cmd, sizeof(cmd), "REGISTER %d", port);
        send_command(server_ip, server_port, cmd);
        printf("Sent registration to server %s:%d\n", server_ip, server_port);
    }

    while (1) {
        struct sockaddr_in sender_addr;
        socklen_t sender_len = sizeof(sender_addr);
        ssize_t len = recvfrom(sockfd, buffer, MAX_MSG_LEN - 1, 0,
                              (struct sockaddr*)&sender_addr, &sender_len);
        if (len > 0) {
            buffer[len] = '\0';
            
            if (strcmp(buffer, "PING") == 0) {
                // Отвечаем на ping
                sendto(sockfd, "PONG", 4, 0, 
                      (struct sockaddr*)&sender_addr, sender_len);
            } else {
                printf("[LOG] %s\n", buffer);
            }
        }
    }

    close(sockfd);
    return 0;
}