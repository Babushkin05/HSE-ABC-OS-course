#include <stdio.h>
#include <unistd.h>
#include <string.h> 
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

// cat /some/file | grep "needle"

char* path = "./test.txt";

int main() {
    int fd[2];
    pipe2(fd, O_CLOEXEC);
    pid_t son_id = fork();
    if(son_id == 0){
        close(1);
        dup2(fd[1],1);
        int ffd = open(path, 0, 0666);
        char buf[100];
        int readed = read(ffd, buf, 100);
        close(fd[0]);
        write(fd[1], buf, strlen(buf) + 1);
        close(fd[1]);
    }
    else{
        wait(NULL);
        char message[100];
        read(fd[0], message, 100);
        close(fd[0]);
        char ans[100];
        execl("grep", "grep", "needle", NULL);
    }
}