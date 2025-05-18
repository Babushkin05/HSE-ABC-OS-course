#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>

#define NUM_PAINTINGS 5
#define BUFFER_SIZE 1024
#define MAX_RETRIES 5
#define MAX_ATTEMPTS 3
#define RETRY_DELAY 1
#define READ_TIMEOUT_SEC 5
#define READ_TIMEOUT_USEC 0

void usage() {
    printf("Usage: ./client <server_ip> <server_port> <visitor_id>\n");
    exit(1);
}

void handle_error(int sock) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "-1 EXIT -1");
    send(sock, buffer, strlen(buffer), 0);
    close(sock);
}

int connect_with_retry(const char* ip, int port, int visitor_id) {
    int sock;
    struct sockaddr_in serv_addr;
    
    for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            continue;
        }
        
        struct timeval timeout;
        timeout.tv_sec = READ_TIMEOUT_SEC;
        timeout.tv_usec = READ_TIMEOUT_USEC;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
            close(sock);
            continue;
        }
        
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0) {
            return sock;
        }
        
        close(sock);
        if (attempt < MAX_RETRIES - 1) {
            sleep(RETRY_DELAY);
        }
    }

    return -1;
}

int send_and_wait_response(int sock, const char* message, char* response, int visitor_id) {
    int attempts = 0;
    
    while (attempts < MAX_ATTEMPTS) {
        if (send(sock, message, strlen(message), 0) < 0) {
            return -1;
        }
        
        ssize_t bytes_read = recv(sock, response, BUFFER_SIZE - 1, 0);
        
        if (bytes_read > 0) {
            response[bytes_read] = '\0';
            return 0;
        } else if (bytes_read == 0) {
            return -1;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                attempts++;
                continue;
            }
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
    
    snprintf(buffer, BUFFER_SIZE, "%d ENTER -1", visitor_id);
    if (send_and_wait_response(sock, buffer, response, visitor_id) != 0) {
        handle_error(sock);
        return;
    }
    
    if (strstr(response, "ENTER_WAIT") != NULL) {
        while (1) {
            sleep(1 + rand() % 2);
            snprintf(buffer, BUFFER_SIZE, "%d ENTER -1", visitor_id);
            if (send_and_wait_response(sock, buffer, response, visitor_id) != 0) {
                handle_error(sock);
                return;
            }
            if (strstr(response, "ENTER_OK") != NULL) break;
        }
    }
    
    while (paintings_to_visit > 0) {
        int painting = rand() % NUM_PAINTINGS;
        
        if (!visited[painting]) {
            snprintf(buffer, BUFFER_SIZE, "%d VIEW %d", visitor_id, painting);
            if (send_and_wait_response(sock, buffer, response, visitor_id) != 0) {
                handle_error(sock);
                return;
            }
            
            if (strstr(response, "VIEW_OK") != NULL) {
                sleep(1 + rand() % 3);
                visited[painting] = 1;
                paintings_to_visit--;
                
                snprintf(buffer, BUFFER_SIZE, "%d LEAVE_PAINTING %d", visitor_id, painting);
                if (send_and_wait_response(sock, buffer, response, visitor_id) != 0) {
                    handle_error(sock);
                    return;
                }
            } else {
                sleep(1);
            }
        }
    }
    
    snprintf(buffer, BUFFER_SIZE, "%d EXIT -1", visitor_id);
    send_and_wait_response(sock, buffer, response, visitor_id);
}

int main(int argc, char *argv[]) {
    if (argc != 4) usage();
    
    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int visitor_id = atoi(argv[3]);
    
    srand(time(NULL) + visitor_id);
    
    int sock = connect_with_retry(server_ip, server_port, visitor_id);
    if (sock < 0) return 1;
    
    visit_gallery(sock, visitor_id);
    
    close(sock);
    return 0;
}