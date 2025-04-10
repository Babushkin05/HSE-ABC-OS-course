#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <time.h>  
#include <stdatomic.h>
#include <sched.h>
#include <unistd.h>

#include "anon-sem.h"

int main() {
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

    int val;
    if (sem_getvalue(&buffer->sem, &val) == -1) {
        perror("sem_getvalue");
        return 1;
    }
    printf("Semaphore value before post = %d\n", val);

    if (sem_post(&buffer->sem) == -1) {
        perror("sem_post");
        return 1;
    };

    printf("sem_post: success\n");

    if (munmap(buffer, sizeof(shared_mem_t)) == -1) {
        perror("munmap");
        return 1;
    }

    close(shmfd);

    return 0;
}

