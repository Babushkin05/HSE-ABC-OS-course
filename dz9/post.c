#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <time.h>  
#include <stdatomic.h>
#include <sched.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "anon-sem.h"

_Bool is_continue = 1;

void handle_signal(int sig) {
    (void)sig;
    is_continue = 0;
}

int main() {

    // handle ctrl + C
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);


    int shmfd = shm_open(SHMEM_NAME, O_RDWR, 0600);
    if (shmfd < 0) {
        perror("shm_open");
        return 1;
    }

    shared_mem_t *buffer = mmap(NULL, sizeof(shared_mem_t),
                                PROT_READ|PROT_WRITE, MAP_SHARED,
                                shmfd, 0);
    if (!buffer) {
        perror("mmap");
        return 1;
    }

    if (!atomic_load(&buffer->mem_ready)) {
        fprintf(stderr, "sem not ready\n");
        return 1;
    }

    while(is_continue){
        time_t now = time(NULL);
        if (now == -1) {
            perror("time");
            break;
        }

        struct tm *local_time = localtime(&now);
        if (local_time == NULL) {
            perror("localtime");
            break;
        }

        char time_str[100];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S\n", local_time);
        if (sem_post(&buffer->sem1) == -1) {
            perror("sem_post");
            break;
        };
        
        strcpy(buffer->time_str, time_str);
        //printf("%s\n", buffer->time_str);

        if (sem_wait(&buffer->sem2) == -1) {
            perror("sem_wait");
            break;
        };

        sleep(1);
    }

    if (munmap(buffer, sizeof(shared_mem_t)) == -1) {
        perror("munmap");
        return 1;
    }

    close(shmfd);

    return 0;
}

