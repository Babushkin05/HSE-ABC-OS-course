#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>    
#include <stdio.h>   
#include <string.h> 
#include <stddef.h>
#include <sys/wait.h>
#include <stdlib.h>


int main(int argc, char** argv) {
    if( argc != 3){
        printf("./test <pid of main program> <time between SIGUSR1>");
        _exit(1);
    }
    while(1){
        kill(atoi(argv[1]), SIGUSR1);
        sleep(atoi(argv[1]));
    }
}