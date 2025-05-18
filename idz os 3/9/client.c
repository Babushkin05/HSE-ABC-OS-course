#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#define NUM_PAINTINGS 5
#define BUFFER_SIZE 1024
#define MAX_RETRIES 5
#define MAX_ATTEMPTS 3
#define RETRY_DELAY 1
#define READ_TIMEOUT_SEC 5
#define READ_TIMEOUT_USEC 0

// Глобальные переменные для доступа из обработчика сигналов
static int g_sock = -1;
static int g_visitor_id = -1;

void usage() {
    printf("Usage: ./client <server_ip> <server_port> <visitor_id>\n");
    exit(EXIT_FAILURE);
}

void cleanup(int sig) {
    if (g_sock != -1) {
        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "%d EXIT -1", g_visitor_id);
        send(g_sock, buffer, strlen(buffer), 0);
        close(g_sock);
        printf("\nVisitor %d: Cleanup complete. Exiting.\n", g_visitor_id);
    }
    exit(EXIT_SUCCESS);
}

void setup_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = cleanup;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Failed to set up SIGINT handler");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Failed to set up SIGTERM handler");
        exit(EXIT_FAILURE);
    }
}

int connect_with_retry(const char* ip, int port) {
    struct sockaddr_in serv_addr;
    int sock;
    
    for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Socket creation error");
            continue;
        }
        
        struct timeval timeout = {
            .tv_sec = READ_TIMEOUT_SEC,
            .tv_usec = READ_TIMEOUT_USEC
        };
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
            perror("Invalid address/Address not supported");
            close(sock);
            continue;
        }
        
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
            perror("Connection failed");
            close(sock);
            if (attempt < MAX_RETRIES - 1) {
                sleep(RETRY_DELAY);
            }
            continue;
        }
        
        return sock;
    }

    return -1;
}

int send_and_wait_response(int sock, const char* message, char* response) {
    int attempts = 0;
    
    while (attempts < MAX_ATTEMPTS) {
        if (send(sock, message, strlen(message), 0) < 0) {
            perror("Send failed");
            return -1;
        }
        
        ssize_t bytes_read = recv(sock, response, BUFFER_SIZE - 1, 0);
        
        if (bytes_read > 0) {
            response[bytes_read] = '\0';
            return 0;
        } else if (bytes_read == 0) {
            printf("Connection closed by server\n");
            return -1;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                attempts++;
                continue;
            }
            perror("Receive failed");
            return -1;
        }
    }
    
    return -1;
}

void visit_gallery(int sock, int visitor_id) {
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int paintings_to_visit = NUM_PAINTINGS;
    int visited[NUM_PAINTINGS] = {0};
    
    // ENTER command
    snprintf(buffer, BUFFER_SIZE, "%d ENTER -1", visitor_id);
    if (send_and_wait_response(sock, buffer, response) != 0) {
        fprintf(stderr, "Failed to enter gallery\n");
        return;
    }
    
    if (strstr(response, "ENTER_WAIT") != NULL) {
        printf("Visitor %d: Waiting to enter gallery...\n", visitor_id);
        while (1) {
            sleep(1 + rand() % 2);
            snprintf(buffer, BUFFER_SIZE, "%d ENTER -1", visitor_id);
            if (send_and_wait_response(sock, buffer, response)) {
                fprintf(stderr, "Connection error while waiting\n");
                return;
            }
            if (strstr(response, "ENTER_OK") != NULL) break;
        }
    }
    
    printf("Visitor %d: Entered gallery\n", visitor_id);
    
    // View paintings
    while (paintings_to_visit > 0) {
        int painting = rand() % NUM_PAINTINGS;
        
        if (!visited[painting]) {
            snprintf(buffer, BUFFER_SIZE, "%d VIEW %d", visitor_id, painting);
            if (send_and_wait_response(sock, buffer, response) != 0) {
                fprintf(stderr, "Failed to view painting %d\n", painting);
                return;
            }
            
            if (strstr(response, "VIEW_OK") != NULL) {
                printf("Visitor %d: Viewing painting %d\n", visitor_id, painting);
                sleep(1 + rand() % 3);
                visited[painting] = 1;
                paintings_to_visit--;
                
                snprintf(buffer, BUFFER_SIZE, "%d LEAVE_PAINTING %d", visitor_id, painting);
                if (send_and_wait_response(sock, buffer, response) != 0) {
                    fprintf(stderr, "Failed to leave painting %d\n", painting);
                    return;
                }
                printf("Visitor %d: Left painting %d\n", visitor_id, painting);
            } else {
                sleep(1);
            }
        }
    }
    
    // EXIT command
    snprintf(buffer, BUFFER_SIZE, "%d EXIT -1", visitor_id);
    send_and_wait_response(sock, buffer, response);
    printf("Visitor %d: Exited gallery\n", visitor_id);
}

int main(int argc, char *argv[]) {
    if (argc != 4) usage();
    
    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);
    g_visitor_id = atoi(argv[3]);
    
    srand(time(NULL) + g_visitor_id);
    setup_signal_handlers();
    
    printf("Visitor %d: Starting...\n", g_visitor_id);
    g_sock = connect_with_retry(server_ip, server_port);
    if (g_sock < 0) {
        fprintf(stderr, "Visitor %d: Failed to connect to server\n", g_visitor_id);
        return EXIT_FAILURE;
    }
    
    printf("Visitor %d: Connected to server\n", g_visitor_id);
    visit_gallery(g_sock, g_visitor_id);
    
    close(g_sock);
    return EXIT_SUCCESS;
}