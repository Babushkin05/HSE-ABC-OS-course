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

typedef struct {
    int sock;
    int visitor_id;
} client_data_t;

volatile sig_atomic_t g_shutdown = 0;
client_data_t g_client;

void usage() {
    printf("Usage: ./client <server_ip> <server_port> <visitor_id>\n");
    exit(EXIT_FAILURE);
}

void handle_signal(int sig) {
    g_shutdown = 1;
}

void setup_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    signal(SIGPIPE, SIG_IGN);
}

void cleanup() {
    if (g_client.sock != -1) {
        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "%d EXIT -1", g_client.visitor_id);
        send(g_client.sock, buffer, strlen(buffer), 0);
        close(g_client.sock);
    }
}

int connect_with_retry(const char* ip, int port) {
    struct sockaddr_in serv_addr;
    
    for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            continue;
        }
        
        struct timeval timeout = {
            .tv_sec = READ_TIMEOUT_SEC,
            .tv_usec = READ_TIMEOUT_USEC
        };
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
            close(sock);
            continue;
        }
        
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
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

int process_server_response(char* response) {
    if (strstr(response, "SERVER_SHUTDOWN") != NULL) {
        return -1;
    }
    return 0;
}

int send_and_wait_response(const char* message, char* response) {
    int attempts = 0;
    
    while (attempts < MAX_ATTEMPTS && !g_shutdown) {
        if (send(g_client.sock, message, strlen(message), 0) < 0) {
            return -1;
        }
        
        ssize_t bytes_read = recv(g_client.sock, response, BUFFER_SIZE - 1, 0);
        
        if (bytes_read > 0) {
            response[bytes_read] = '\0';
            if (process_server_response(response) == -1) return -1;
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

void visit_gallery() {
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int paintings_to_visit = NUM_PAINTINGS;
    int visited[NUM_PAINTINGS] = {0};
    
    snprintf(buffer, BUFFER_SIZE, "%d ENTER -1", g_client.visitor_id);
    if (send_and_wait_response(buffer, response) != 0) {
        return;
    }
    
    if (strstr(response, "ENTER_WAIT") != NULL) {
        while (!g_shutdown) {
            sleep(1 + rand() % 2);
            snprintf(buffer, BUFFER_SIZE, "%d ENTER -1", g_client.visitor_id);
            if (send_and_wait_response(buffer, response)) {
                return;
            }
            if (strstr(response, "ENTER_OK") != NULL) break;
        }
    }
    
    while (paintings_to_visit > 0 && !g_shutdown) {
        int painting = rand() % NUM_PAINTINGS;
        
        if (!visited[painting]) {
            snprintf(buffer, BUFFER_SIZE, "%d VIEW %d", g_client.visitor_id, painting);
            if (send_and_wait_response(buffer, response)) {
                return;
            }
            
            if (strstr(response, "VIEW_OK") != NULL) {
                sleep(1 + rand() % 3);
                visited[painting] = 1;
                paintings_to_visit--;
                
                snprintf(buffer, BUFFER_SIZE, "%d LEAVE_PAINTING %d", g_client.visitor_id, painting);
                if (send_and_wait_response(buffer, response)) {
                    return;
                }
            } else {
                sleep(1);
            }
        }
    }
    
    if (!g_shutdown) {
        snprintf(buffer, BUFFER_SIZE, "%d EXIT -1", g_client.visitor_id);
        send_and_wait_response(buffer, response);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) usage();
    
    setup_signal_handlers();
    atexit(cleanup);
    
    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);
    g_client.visitor_id = atoi(argv[3]);
    g_client.sock = -1;
    
    srand(time(NULL) + g_client.visitor_id);
    
    g_client.sock = connect_with_retry(server_ip, server_port);
    if (g_client.sock < 0) {
        return EXIT_FAILURE;
    }
    
    visit_gallery();
    return EXIT_SUCCESS;
}