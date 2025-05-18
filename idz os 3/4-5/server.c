#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_VISITORS 25
#define MAX_PAINTING_VISITORS 5
#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[]) {
    int port = DEFAULT_PORT;
    
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    
    // Создание сокета
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Настройка сокета
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    // Привязка сокета к порту
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Ожидание подключений
    #define MAX_PENDING_CONNECTIONS 100
    if (listen(server_fd, MAX_PENDING_CONNECTIONS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Сервер (вахтер) запущен на порту %d и ожидает подключений...\n", port);
    
    int current_visitors = 0;
    int painting_counts[MAX_PAINTING_VISITORS] = {0}; 
    
    while (1) {
        // Принятие нового подключения
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
        // Чтение сообщения от клиента
        read(new_socket, buffer, BUFFER_SIZE);
        printf("Получено от клиента: %s\n", buffer);
        
        // Обработка сообщения
        char response[BUFFER_SIZE] = {0};
        int visitor_pid, painting_num;
        char action[20];
        
        sscanf(buffer, "%d %s %d", &visitor_pid, action, &painting_num);
        
        if (strcmp(action, "ENTER") == 0) {
            if (current_visitors < MAX_VISITORS) {
                current_visitors++;
                sprintf(response, "ENTER_OK %d", visitor_pid);
            } else {
                sprintf(response, "ENTER_WAIT %d", visitor_pid);
            }
        } 
        else if (strcmp(action, "VIEW") == 0) {
            if (painting_num >= 0 && painting_num < 5) {
                if (painting_counts[painting_num] < MAX_PAINTING_VISITORS) {
                    painting_counts[painting_num]++;
                    sprintf(response, "VIEW_OK %d %d", visitor_pid, painting_num);
                } else {
                    sprintf(response, "VIEW_WAIT %d %d", visitor_pid, painting_num);
                }
            }
        }
        else if (strcmp(action, "LEAVE_PAINTING") == 0) {
            if (painting_num >= 0 && painting_num < 5 && painting_counts[painting_num] > 0) {
                painting_counts[painting_num]--;
                sprintf(response, "LEFT_PAINTING %d %d", visitor_pid, painting_num);
            }
        }
        else if (strcmp(action, "EXIT") == 0) {
            current_visitors--;
            sprintf(response, "EXIT_OK %d", visitor_pid);
        }
        
        // Отправка ответа клиенту
        send(new_socket, response, strlen(response), 0);
        
        close(new_socket);
        memset(buffer, 0, BUFFER_SIZE);
    }
    
    return 0;
}