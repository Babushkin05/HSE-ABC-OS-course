#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXPENDING 5

void DieWithError(char *errorMessage);

int main(int argc, char *argv[]) {
    int servSock, clntSock1, clntSock2;
    struct sockaddr_in echoServAddr, echoClntAddr;
    unsigned short echoServPort;
    unsigned int clntLen;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);

    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    echoServAddr.sin_port = htons(echoServPort);

    if (bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");

    if (listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed");

    printf("waiting  clients...\n");

    clntLen = sizeof(echoClntAddr);
    if ((clntSock1 = accept(servSock, (struct sockaddr *)&echoClntAddr, &clntLen)) < 0)
        DieWithError("accept() failed for client 1");
    printf("client 1 connected: %s\n", inet_ntoa(echoClntAddr.sin_addr));

    if ((clntSock2 = accept(servSock, (struct sockaddr *)&echoClntAddr, &clntLen)) < 0)
        DieWithError("accept() failed for client 2");
    printf("client 2 connected: %s\n", inet_ntoa(echoClntAddr.sin_addr));

    char buffer[256];
    int recvMsgSize;

    while ((recvMsgSize = recv(clntSock1, buffer, sizeof(buffer), 0)) > 0) {
        if (strcmp(buffer, "The End") == 0) break;
        send(clntSock2, buffer, recvMsgSize, 0);
    }

    close(clntSock1);
    close(clntSock2);
    close(servSock);
    return 0;
}

void DieWithError(char *errorMessage) {
    perror(errorMessage);
    exit(1);
}