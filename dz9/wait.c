#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <stdatomic.h>
#include <unistd.h> 

#include "anon-sem.h"

int main() {
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

    sem_init(&buffer->sem, 1 /*true*/, 0);
    atomic_store(&buffer->mem_ready, 1);

    int val;
    if (sem_getvalue(&buffer->sem, &val) == -1) {
        perror("sem_getvalue");
        return 1;
    }
    printf("Semaphore value before wait = %d\n", val);

    // Should block
    if (sem_wait(&buffer->sem) == -1) {
        perror("sem_wait");
        return 1;
    };

    printf("sem_wait returned successfully\n");

    if (sem_destroy(&buffer->sem) == -1) {
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

