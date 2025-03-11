#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Структура сообщения
struct msg_buffer {
    long mtype;          
    char mtext[128];    
};

int main(int argc, char** argv) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <from> <to> <msgq1> <msgq2>\n", argv[0]);
        exit(1);
    }

    char* from = argv[1];
    char* to = argv[2];
    key_t key1 = ftok(argv[3], 65); 
    key_t key2 = ftok(argv[4], 65);

    if (key1 == -1 || key2 == -1) {
        perror("ftok");
        exit(1);
    }

    int msgq1 = msgget(key1, 0666 | IPC_CREAT);
    int msgq2 = msgget(key2, 0666 | IPC_CREAT);

    if (msgq1 == -1 || msgq2 == -1) {
        perror("msgget");
        exit(1);
    }

    int fd = open(from, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    struct msg_buffer msg;
    msg.mtype = 1;  

    ssize_t bytes_read;
    while ((bytes_read = read(fd, msg.mtext, sizeof(msg.mtext))) > 0) {
        if (bytes_read == -1) {
            perror("read");
            close(fd);
            exit(1);
        }

        if (msgsnd(msgq1, &msg, bytes_read, 0) == -1) {
            perror("msgsnd");
            close(fd);
            exit(1);
        }
    }

    if (close(fd) == -1) {
        perror("close");
        exit(1);
    }

    ssize_t bytes_received;
    int out_fd = creat(to, 0666);
    if (out_fd == -1) {
        perror("creat");
        exit(1);
    }

    while ((bytes_received = msgrcv(msgq2, &msg, sizeof(msg.mtext), 1, 0)) > 0) {
        int flag = 0;
        char* endstr = memchr(msg.mtext, '\0', bytes_received);
        if(endstr != NULL){
            bytes_received = endstr - msg.mtext;
            flag = 1;
        }
        if (write(out_fd, msg.mtext, bytes_received) == -1) {
            perror("write");
            close(out_fd);
            exit(1);
        }
        if(flag){
            break;
        }
    }

    if (close(out_fd) == -1) {
        perror("close");
        exit(1);
    }

    if (msgctl(msgq1, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    if (msgctl(msgq2, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    return 0;
}