#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>  
#include <stdatomic.h>
#include <sched.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "shared_mem.h"

#define NUM_PROGS 3
#define MAX_TASKS 50
#define CORRECT_PROBABILITY 70  // 70% chance program is correct

static _Atomic volatile sig_atomic_t unlink_done = 0;

shared_mem_t *buffer;

void sigint_handler(int sig) {
    if (atomic_load(&unlink_done)) {
        return;  
    }

    if (!atomic_load(&unlink_done)) {
        for (int i = 0; i < NUM_PROGS; i++) {
            sem_destroy(&buffer->task_sems[i]);
        }
        sem_destroy(&buffer->mutex);

        atomic_store(&unlink_done, 1); 
    }

    kill(0, SIGINT);
    _exit(EXIT_SUCCESS); 
}

void add_task(shared_mem_t *buffer, size_t proger_num, int task) {
    int *queue;
    size_t *index;
    
    switch(proger_num) {
        case 0:
            queue = buffer->queue1;
            index = &buffer->i;
            break;
        case 1:
            queue = buffer->queue2;
            index = &buffer->j;
            break;
        case 2:
            queue = buffer->queue3;
            index = &buffer->k;
            break;
        default:
            return;
    }
    
    if (*index < MAX_TASKS) {
        queue[(*index)++] = task;
    }
}

int get_task(shared_mem_t *buffer, size_t proger_num) {
    int *queue;
    size_t *index;
    
    switch(proger_num) {
        case 0:
            queue = buffer->queue1;
            index = &buffer->i;
            break;
        case 1:
            queue = buffer->queue2;
            index = &buffer->j;
            break;
        case 2:
            queue = buffer->queue3;
            index = &buffer->k;
            break;
        default:
            return -1;
    }
    
    if (*index > 0) {
        int task = queue[0];
        for (size_t i = 1; i < *index; i++) {
            queue[i-1] = queue[i];
        }
        (*index)--;
        return task;
    }
    return -1;
}

void run(size_t proger_num) {
    int shmfd = shm_open(SHMEM_NAME, O_RDWR, 0600);
    if (shmfd < 0) {
        perror("shm_open in run");
        exit(EXIT_FAILURE);
    }

    shared_mem_t *buffer = mmap(NULL, sizeof(shared_mem_t),
                                PROT_READ|PROT_WRITE, MAP_SHARED,
                                shmfd, 0);
    if (!buffer) {
        perror("mmap in run");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL) + proger_num);
    
    while (1) {
        sem_wait(&buffer->task_sems[proger_num]);
        
        sem_wait(&buffer->mutex);
        int task = get_task(buffer, proger_num);
        sem_post(&buffer->mutex);
        
        if (task == -1) {
            printf("Programmer %zu is writing his own program\n", proger_num);
            usleep(500000 + rand() % 500000);
            
            size_t reviewer;
            do {
                reviewer = rand() % NUM_PROGS;
            } while (reviewer == proger_num);
            
            printf("Programmer %zu sends his program to programmer %zu for review\n", 
                   proger_num, reviewer);
            
            sem_wait(&buffer->mutex);
            add_task(buffer, reviewer, proger_num);
            sem_post(&buffer->mutex);
            
            sem_post(&buffer->task_sems[reviewer]);
        } 
        else if (task >= 0 && task < NUM_PROGS) {
            printf("Programmer %zu is reviewing programmer %d's program\n", 
                   proger_num, task);
            usleep(800000 + rand() % 600000);
            
            int is_correct = (rand() % 100) < CORRECT_PROBABILITY;
            
            if (is_correct) {
                printf("Programmer %zu says programmer %d's program is CORRECT\n", 
                       proger_num, task);
                
                sem_wait(&buffer->mutex);
                add_task(buffer, task, -1);
                sem_post(&buffer->mutex);
                
                sem_post(&buffer->task_sems[task]);
            } else {
                printf("Programmer %zu says programmer %d's program is INCORRECT\n", 
                       proger_num, task);
                
                sem_wait(&buffer->mutex);
                add_task(buffer, proger_num, task + NUM_PROGS);
                sem_post(&buffer->mutex);
                
                sem_post(&buffer->task_sems[proger_num]);
            }
        } 
        else if (task >= NUM_PROGS && task < 2*NUM_PROGS) {
            int original_proger = task - NUM_PROGS;
            printf("Programmer %zu is re-reviewing programmer %d's program\n", 
                   proger_num, original_proger);
            usleep(500000 + rand() % 300000);
            
            int is_correct = (rand() % 100) < 85;
            
            if (is_correct) {
                printf("Programmer %zu says programmer %d's program is NOW CORRECT\n", 
                       proger_num, original_proger);
                
                sem_wait(&buffer->mutex);
                add_task(buffer, original_proger, -1);
                sem_post(&buffer->mutex);
                
                sem_post(&buffer->task_sems[original_proger]);
            } else {
                printf("Programmer %zu says programmer %d's program is STILL INCORRECT\n", 
                       proger_num, original_proger);
                
                sem_wait(&buffer->mutex);
                add_task(buffer, proger_num, task);
                sem_post(&buffer->mutex);
                
                sem_post(&buffer->task_sems[proger_num]);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <programmer_id>\n", argv[0]);
        fprintf(stderr, "       programmer_id can be 0, 1, 2 or -1 for initializer\n");
        return EXIT_FAILURE;
    }

    int proger_num = atoi(argv[1]);
    if (proger_num < -1 || proger_num >= NUM_PROGS) {
        fprintf(stderr, "Invalid programmer ID\n");
        return EXIT_FAILURE;
    }

    // Set up signal handler
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    if (proger_num == -1) {
        // Initializer process - sets up shared memory
        shm_unlink(SHMEM_NAME);

        int shmfd = shm_open(SHMEM_NAME, O_CREAT|O_RDWR, 0600);
        if (shmfd < 0) {
            perror("shm_open");
            return EXIT_FAILURE;
        }

        if (ftruncate(shmfd, sizeof(shared_mem_t))) {
            perror("ftruncate");
            return EXIT_FAILURE;
        }

        buffer = mmap(NULL, sizeof(shared_mem_t),
                                    PROT_READ|PROT_WRITE, MAP_SHARED,
                                    shmfd, 0);
        if (!buffer) {
            perror("mmap");
            return EXIT_FAILURE;
        }

        // Initialize shared memory
        buffer->i = 0;
        buffer->j = 0;
        buffer->k = 0;
        memset(buffer->queue1, -1, sizeof(buffer->queue1));
        memset(buffer->queue2, -1, sizeof(buffer->queue2));
        memset(buffer->queue3, -1, sizeof(buffer->queue3));

        if (sem_init(&buffer->mutex, 1, 1)) {
            perror("sem_init(mutex)");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < NUM_PROGS; i++) {
            if (sem_init(&buffer->task_sems[i], 1, 0)) {
                perror("sem_init(sem)");
                for (int j = 0; j < i; j++) {
                    sem_destroy(&buffer->task_sems[j]);
                }
                sem_destroy(&buffer->mutex);
                
                exit(EXIT_FAILURE);
            }
        }

        // Add initial tasks
        for (int i = 0; i < NUM_PROGS; i++) {
            add_task(buffer, i, -1);
            sem_post(&buffer->task_sems[i]);
        }

        printf("Shared memory initialized. You can now run programmers.\n");
        pause(); // Wait for SIGINT
    } else {
        // Regular programmer process
        run(proger_num);
    }

    return EXIT_SUCCESS;
}