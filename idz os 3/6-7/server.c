#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>
#include <sys/select.h>
#include <errno.h>

#define MAX_VISITORS 25
#define MAX_PAINTING_VISITORS 5
#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024
#define MAX_PENDING_CONNECTIONS 100
#define MAX_CLIENTS 300

typedef struct {
    int fd;
    int visitor_id;
    int current_painting;
    int is_monitor;
    int sent_exit; // Флаг, что команда EXIT уже отправлена
} client_t;

client_t clients[MAX_CLIENTS];
int current_visitors = 0;
int painting_counts[5] = {0};

void broadcast_status() {
    char status[BUFFER_SIZE];
    snprintf(status, BUFFER_SIZE, "%d %d %d %d %d %d ", 
             current_visitors,
             painting_counts[0],
             painting_counts[1],
             painting_counts[2],
             painting_counts[3],
             painting_counts[4]);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd != -1 && clients[i].is_monitor) {
            send(clients[i].fd, status, strlen(status), 0);
        }
    }
}

void handle_client_message(int client_idx) {
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};
    int bytes_read = read(clients[client_idx].fd, buffer, BUFFER_SIZE - 1);
    
    if (bytes_read <= 0) {
        // Обработка разрыва соединения
        if (clients[client_idx].current_painting != -1) {
            painting_counts[clients[client_idx].current_painting] = 
                (painting_counts[clients[client_idx].current_painting] > 0) ? 
                painting_counts[clients[client_idx].current_painting] - 1 : 0;
        }
        
        // Уменьшаем счетчик только если не было команды EXIT
        if (clients[client_idx].visitor_id != -1 && 
            !clients[client_idx].is_monitor && 
            !clients[client_idx].sent_exit) {
            current_visitors = (current_visitors > 0) ? current_visitors - 1 : 0;
        }
        
        close(clients[client_idx].fd);
        clients[client_idx].fd = -1;
        broadcast_status();
        return;
    }
    
    buffer[bytes_read] = '\0';
    
    int visitor_id, painting_num;
    char action[20];
    
    if (sscanf(buffer, "%d %s %d", &visitor_id, action, &painting_num) != 3) {
        if (strcmp(buffer, "MONITOR") == 0) {
            clients[client_idx].is_monitor = 1;
            broadcast_status();
        }
        return;
    }
    
    clients[client_idx].visitor_id = visitor_id;
    
    if (strcmp(action, "ENTER") == 0) {
        if (current_visitors < MAX_VISITORS) {
            current_visitors++;
            snprintf(response, BUFFER_SIZE, "ENTER_OK %d", visitor_id);
        } else {
            snprintf(response, BUFFER_SIZE, "ENTER_WAIT %d", visitor_id);
        }
        broadcast_status();
    } 
    else if (strcmp(action, "VIEW") == 0) {
        if (painting_num >= 0 && painting_num < 5) {
            if (painting_counts[painting_num] < MAX_PAINTING_VISITORS) {
                painting_counts[painting_num]++;
                clients[client_idx].current_painting = painting_num;
                snprintf(response, BUFFER_SIZE, "VIEW_OK %d %d", visitor_id, painting_num);
            } else {
                snprintf(response, BUFFER_SIZE, "VIEW_WAIT %d %d", visitor_id, painting_num);
            }
            broadcast_status();
        }
    }
    else if (strcmp(action, "LEAVE_PAINTING") == 0) {
        if (clients[client_idx].current_painting != -1) {
            painting_counts[clients[client_idx].current_painting] = 
                (painting_counts[clients[client_idx].current_painting] > 0) ? 
                painting_counts[clients[client_idx].current_painting] - 1 : 0;
            clients[client_idx].current_painting = -1;
            snprintf(response, BUFFER_SIZE, "LEFT_PAINTING %d", visitor_id);
            broadcast_status();
        }
    }
    else if (strcmp(action, "EXIT") == 0) {
        if (clients[client_idx].current_painting != -1) {
            painting_counts[clients[client_idx].current_painting] = 
                (painting_counts[clients[client_idx].current_painting] > 0) ? 
                painting_counts[clients[client_idx].current_painting] - 1 : 0;
            clients[client_idx].current_painting = -1;
        }
        current_visitors = (current_visitors > 0) ? current_visitors - 1 : 0;
        clients[client_idx].sent_exit = 1; // Устанавливаем флаг
        snprintf(response, BUFFER_SIZE, "EXIT_OK %d", visitor_id);
        broadcast_status();
    }
    
    if (strlen(response) > 0) {
        send(clients[client_idx].fd, response, strlen(response), 0);
    }
}

int main(int argc, char const *argv[]) {
    int port = DEFAULT_PORT;
    if (argc > 1) port = atoi(argv[1]);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].visitor_id = -1;
        clients[i].current_painting = -1;
        clients[i].is_monitor = 0;
        clients[i].sent_exit = 0;
    }
    
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, MAX_PENDING_CONNECTIONS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Server started on port %d\n", port);
    
    while (1) {
        fd_set readfds;
        int max_fd = server_fd;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd > 0) {
                FD_SET(clients[i].fd, &readfds);
                if (clients[i].fd > max_fd) max_fd = clients[i].fd;
            }
        }
        
        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }
        
        if (FD_ISSET(server_fd, &readfds)) {
            struct sockaddr_in client_addr;
            int addrlen = sizeof(client_addr);
            int new_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen);
            
            if (new_socket < 0) {
                perror("accept");
                continue;
            }
            
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].fd == -1) {
                    clients[i].fd = new_socket;
                    break;
                }
            }
        }
        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd > 0 && FD_ISSET(clients[i].fd, &readfds)) {
                handle_client_message(i);
            }
        }
    }
    
    close(server_fd);
    return 0;
}