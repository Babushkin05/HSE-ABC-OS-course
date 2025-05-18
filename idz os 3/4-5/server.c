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
} client_t;

client_t clients[MAX_CLIENTS];
int current_visitors = 0;
int painting_counts[5] = {0};

void handle_client_message(int client_idx) {
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};
    int bytes_read;
    
    bytes_read = read(clients[client_idx].fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read <= 0) {
        if (bytes_read == 0) {
        } else {
            perror("read error");
        }
        
        // Free resources
        if (clients[client_idx].current_painting != -1) {
            painting_counts[clients[client_idx].current_painting]--;
        }
        if (clients[client_idx].visitor_id != -1) {
            current_visitors--;
        }
        
        close(clients[client_idx].fd);
        clients[client_idx].fd = -1;
        return;
    }
    
    buffer[bytes_read] = '\0';
    printf("Received from %d: %s\n", clients[client_idx].visitor_id, buffer);
    
    int visitor_id, painting_num;
    char action[20];
    
    if (sscanf(buffer, "%d %s %d", &visitor_id, action, &painting_num) != 3) {
        strcpy(response, "ERROR Invalid format");
        send(clients[client_idx].fd, response, strlen(response), 0);
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
        } else {
            snprintf(response, BUFFER_SIZE, "ERROR Invalid painting");
        }
    }
    else if (strcmp(action, "LEAVE_PAINTING") == 0) {
        if (clients[client_idx].current_painting != -1) {
            painting_counts[clients[client_idx].current_painting]--;
            clients[client_idx].current_painting = -1;
            snprintf(response, BUFFER_SIZE, "LEFT_PAINTING %d", visitor_id);
        } else {
            snprintf(response, BUFFER_SIZE, "ERROR Not viewing");
        }
    }
    else if (strcmp(action, "EXIT") == 0) {
        current_visitors--;
        snprintf(response, BUFFER_SIZE, "EXIT_OK %d", visitor_id);
    } else {
        snprintf(response, BUFFER_SIZE, "ERROR Unknown command");
    }
    
    send(clients[client_idx].fd, response, strlen(response), 0);
}

int main(int argc, char const *argv[]) {
    int port = DEFAULT_PORT;
    if (argc > 1) port = atoi(argv[1]);
    
    // Initialize clients
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].visitor_id = -1;
        clients[i].current_painting = -1;
    }
    
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen
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
        
        // Add client sockets
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd > 0) {
                FD_SET(clients[i].fd, &readfds);
                if (clients[i].fd > max_fd) max_fd = clients[i].fd;
            }
        }
        
        // Wait for activity
        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }
        
        // New connection
        if (FD_ISSET(server_fd, &readfds)) {
            struct sockaddr_in client_addr;
            int addrlen = sizeof(client_addr);
            int new_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen);
            
            if (new_socket < 0) {
                perror("accept");
                continue;
            }
            
            // Add new client
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].fd == -1) {
                    clients[i].fd = new_socket;
                    break;
                }
            }
        }
        
        // Handle client messages
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd > 0 && FD_ISSET(clients[i].fd, &readfds)) {
                handle_client_message(i);
            }
        }
    }
    
    close(server_fd);
    return 0;
}