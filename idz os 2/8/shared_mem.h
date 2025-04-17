#pragma once

#define SHMEM_NAME "/my-mem"

typedef struct {
    size_t i;
    size_t j;
    size_t k;
    int queue1[50];
    int queue2[50];
    int queue3[50];
} shared_mem_t;