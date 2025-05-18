#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define NUM_PAINTINGS 5
#define BUFFER_SIZE 1024

void usage() {
    printf("Использование: ./client <server_ip> <server_port> <visitor_id>\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        usage();
    }
    
    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int visitor_id = atoi(argv[3]);
    
    srand(time(NULL) + visitor_id); // Уникальное seed для каждого посетителя
    
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};
    
    // Создание сокета
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    
    // Преобразование IP-адреса
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    
    // Подключение к серверу
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }
    
    
    // Попытка войти в галерею
    sprintf(buffer, "%d ENTER -1", visitor_id);
    send(sock, buffer, strlen(buffer), 0);
    
    read(sock, response, BUFFER_SIZE);
    printf("Посетитель %d получил ответ: %s\n", visitor_id, response);
    
    if (strstr(response, "ENTER_OK") != NULL) {
        int paintings_to_visit = NUM_PAINTINGS;
        int visited[NUM_PAINTINGS] = {0};
        
        while (paintings_to_visit > 0) {
            int painting = rand() % NUM_PAINTINGS;
            
            if (!visited[painting]) {
                memset(buffer, 0, BUFFER_SIZE);
                sprintf(buffer, "%d VIEW %d", visitor_id, painting);
                send(sock, buffer, strlen(buffer), 0);
                
                memset(response, 0, BUFFER_SIZE);
                read(sock, response, BUFFER_SIZE);
                printf("Посетитель %d получил ответ: %s\n", visitor_id, response);
                
                if (strstr(response, "VIEW_OK") != NULL) {
                    sleep(rand() % 3 + 1);
                    visited[painting] = 1;
                    paintings_to_visit--;
                    
                    memset(buffer, 0, BUFFER_SIZE);
                    sprintf(buffer, "%d LEAVE_PAINTING %d", visitor_id, painting);
                    send(sock, buffer, strlen(buffer), 0);
                    
                    memset(response, 0, BUFFER_SIZE);
                    read(sock, response, BUFFER_SIZE);
                    printf("Посетитель %d получил ответ: %s\n", visitor_id, response);
                } else {
                    sleep(1);
                }
            }
        }
        
        memset(buffer, 0, BUFFER_SIZE);
        sprintf(buffer, "%d EXIT -1", visitor_id);
        send(sock, buffer, strlen(buffer), 0);
        
        memset(response, 0, BUFFER_SIZE);
        read(sock, response, BUFFER_SIZE);
        printf("Посетитель %d получил ответ: %s\n", visitor_id, response);
    } else {
        while (strstr(response, "ENTER_OK") == NULL) {
            sleep(1);
            memset(buffer, 0, BUFFER_SIZE);
            sprintf(buffer, "%d ENTER -1", visitor_id);
            send(sock, buffer, strlen(buffer), 0);
            
            memset(response, 0, BUFFER_SIZE);
            read(sock, response, BUFFER_SIZE);
            printf("Посетитель %d получил ответ: %s\n", visitor_id, response);
        }
        
        int paintings_to_visit = NUM_PAINTINGS;
        int visited[NUM_PAINTINGS] = {0};
        
        while (paintings_to_visit > 0) {
            int painting = rand() % NUM_PAINTINGS;
            
            if (!visited[painting]) {
                memset(buffer, 0, BUFFER_SIZE);
                sprintf(buffer, "%d VIEW %d", visitor_id, painting);
                send(sock, buffer, strlen(buffer), 0);
                
                memset(response, 0, BUFFER_SIZE);
                read(sock, response, BUFFER_SIZE);
                printf("Посетитель %d получил ответ: %s\n", visitor_id, response);
                
                if (strstr(response, "VIEW_OK") != NULL) {
                    sleep(rand() % 3 + 1);
                    visited[painting] = 1;
                    paintings_to_visit--;
                    
                    memset(buffer, 0, BUFFER_SIZE);
                    sprintf(buffer, "%d LEAVE_PAINTING %d", visitor_id, painting);
                    send(sock, buffer, strlen(buffer), 0);
                    
                    memset(response, 0, BUFFER_SIZE);
                    read(sock, response, BUFFER_SIZE);
                    printf("Посетитель %d получил ответ: %s\n", visitor_id, response);
                } else {
                    sleep(1);
                }
            }
        }
        
        memset(buffer, 0, BUFFER_SIZE);
        sprintf(buffer, "%d EXIT -1", visitor_id);
        send(sock, buffer, strlen(buffer), 0);
        
        memset(response, 0, BUFFER_SIZE);
        read(sock, response, BUFFER_SIZE);
        printf("Посетитель %d получил ответ: %s\n", visitor_id, response);
    }
    
    close(sock);
    return 0;
}