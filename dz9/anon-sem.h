#pragma once

#include <semaphore.h>

#define SHMEM_NAME "/my-mem"

typedef struct {
    int _Atomic mem_ready;
    char time_str[100];
    sem_t sem1;
    sem_t sem2;
} shared_mem_t;
