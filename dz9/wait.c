#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <stdatomic.h>
#include <unistd.h> 
#include <signal.h>

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


    int shmfd = shm_open(SHMEM_NAME, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (shmfd < 0) {
        perror("shm_open");
        return 1;
    }

    if (ftruncate(shmfd, sizeof(shared_mem_t)) < 0) {
        perror("ftruncate");
        return 1;
    }

    shared_mem_t *buffer = mmap(NULL, sizeof(shared_mem_t),
                                PROT_READ|PROT_WRITE, MAP_SHARED,
                                shmfd, 0);
    if (!buffer) {
        perror("mmap");
        return 1;
    }

    sem_init(&buffer->sem1, 1 /*true*/, 0);
    sem_init(&buffer->sem2, 1 /*true*/, 0);
    atomic_store(&buffer->mem_ready, 1);

    while(is_continue) {
        if (sem_wait(&buffer->sem1) == -1) {
            perror("sem_wait");
            break;
        };

        printf("%s\n", buffer->time_str);

        if (sem_post(&buffer->sem2) == -1) {
            perror("sem_post");
            break;
        };
    }

    if (sem_destroy(&buffer->sem1) == -1) {
        perror("sem_destroy");
        return 1;
    };
    if (sem_destroy(&buffer->sem2) == -1) {
        perror("sem_destroy");
        return 1;
    };

    if (munmap(buffer, sizeof(shared_mem_t)) == -1) {
        perror("munmap");
        return 1;
    }

    close(shmfd);

    if (shm_unlink(SHMEM_NAME) < 0) {
        perror("shm_unlink");
        return 1;
    }

    return 0;
}

