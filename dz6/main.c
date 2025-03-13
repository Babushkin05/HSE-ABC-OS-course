#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>    
#include <stdio.h>   
#include <string.h> 
#include <stddef.h>
#include <sys/wait.h>

volatile sig_atomic_t _Atomic is_log1 = 1;

volatile sig_atomic_t _Atomic fd;

void sigint_handler(int sign, siginfo_t *info, void *context) {
    char* str = "program has stopped by SIGINT";
    if(write(fd, str, strlen(str)) < 0){ // write is signal safety
        // perror("write"); not signal safety
        _exit(1);
    }
    if(close(fd) < 0){ // close is signal safety
        //perror("close"); not signal safety
        _exit(1);
    }
    _exit(0);
}

void sigusr1_handler(int sign, siginfo_t * info, void *context) {
    if (is_log1){
        fd = open("logs-2", O_WRONLY |O_CREAT | O_TRUNC, 0666); // open is signal safety
        if(fd < 0){
            // perror("open"); not signal safety
            _exit(1); // signal safety
        }
        is_log1 = 0;
    }
    else {
        fd = open("logs-1", O_WRONLY |O_CREAT | O_TRUNC, 0666); // open is signal safety
        if(fd < 0){
            // perror("open"); not signal safety
            _exit(1); // signal safety
        }
        is_log1 = 1;
    }
}

int main() {

    printf("use this pid to send signals: %d\n", getpid());

    struct sigaction act = { 0 };

    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &sigint_handler;
    if (sigaction(SIGINT, &act, NULL) == -1) {
        perror("sigaction");
        _exit(1);
    }

    struct sigaction act2 = { 0 };

    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &sigusr1_handler;
    if (sigaction(SIGUSR1, &act, NULL) == -1) {
        perror("sigaction");
        _exit(1);
    }

    fd = open("logs-1", O_WRONLY |O_CREAT | O_TRUNC, 0666);
    if(fd < 0 ){
        perror("open");
        return 1;
    }
    while(1){
        time_t now = time(NULL);
        if (now == -1) {
            perror("time");
            close(fd);
            return 1;
        }

        struct tm *local_time = localtime(&now);
        if (local_time == NULL) {
            perror("localtime");
            close(fd);
            return 1;
        }

        char time_str[100];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S\n", local_time);

        ssize_t bytes_written = write(fd, time_str, strlen(time_str));
        if (bytes_written == -1) {
            perror("write");
            close(fd);
            return 1;
        }

        sleep(1);
    }
}