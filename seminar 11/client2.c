#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RCVBUFSIZE 32

void DieWithError(char *errorMessage);

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in echoServAddr;
    unsigned short echoServPort;
    char *servIP;
    char echoBuffer[RCVBUFSIZE];
    int bytesRcvd;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];
    echoServPort = atoi(argv[2]);

    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);
    echoServAddr.sin_port = htons(echoServPort);

    if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("connect() failed");

    printf("connected to server. Waiting for messages...\n");

    while ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) > 0) {
        echoBuffer[bytesRcvd] = '\0';
        printf("Received: %s\n", echoBuffer);
        if (strcmp(echoBuffer, "The End") == 0) break;
    }

    close(sock);
    return 0;
}

void DieWithError(char *errorMessage) {
    perror(errorMessage);
    exit(1);
}