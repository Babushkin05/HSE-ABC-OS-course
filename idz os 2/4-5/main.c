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

#include "shared_mem.h"

#define NUM_PROGS 3
#define MAX_TASKS 50
#define CORRECT_PROBABILITY 70  // 70% chance program is correct

static _Atomic volatile sig_atomic_t unlink_done = 0;
const char* shared_mem_mutex = "/shared_mem_mutex";
const char* sem_names[NUM_PROGS] = {"/sem1", "/sem2", "/sem3"};

sem_t* mutex = NULL;
sem_t* sems[NUM_PROGS] = {NULL};

void sigint_handler(int sig) {
    if (atomic_load(&unlink_done)) {
        return;  
    }

    if (!atomic_load(&unlink_done)) {
        for (int i = 0; i < NUM_PROGS; i++) {
            sem_close(sems[i]);
            sem_unlink(sem_names[i]);
        }
        sem_close(mutex);
        sem_unlink(shared_mem_mutex);

        atomic_store(&unlink_done, 1); 
    }

    kill(0, SIGINT);
    _exit(EXIT_SUCCESS); 
}

// Add task to programmer's queue
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

// Get task from programmer's queue
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
        // Shift remaining tasks
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

    srand(time(NULL) + proger_num);  // Seed for random numbers
    
    while (1) {
        // 1. Check if there are any tasks in my queue
        sem_wait(sems[proger_num]);
        
        sem_wait(mutex);
        int task = get_task(buffer, proger_num);
        sem_post(mutex);
        
        if (task == -1) {  // new task
            printf("Programmer %zu is writing his own program\n", proger_num);
            usleep(500000 + rand() % 500000);  // Simulate writing
            
            // Randomly select another programmer to review
            size_t reviewer;
            do {
                reviewer = rand() % NUM_PROGS;
            } while (reviewer == proger_num);
            
            printf("Programmer %zu sends his program to programmer %zu for review\n", 
                   proger_num, reviewer);
            
            sem_wait(mutex);
            add_task(buffer, reviewer, proger_num);  // Send task to reviewer
            sem_post(mutex);
            
            sem_post(sems[reviewer]);  // Notify reviewer
        } 
        else if (task >= 0 && task < NUM_PROGS) {  // Need to review someone's program
            printf("Programmer %zu is reviewing programmer %d's program\n", 
                   proger_num, task);
            usleep(800000 + rand() % 600000);  // Simulate reviewing
            
            // Randomly decide if program is correct (70% chance)
            int is_correct = (rand() % 100) < CORRECT_PROBABILITY;
            
            if (is_correct) {
                printf("Programmer %zu says programmer %d's program is CORRECT\n", 
                       proger_num, task);
                
                // Programmer can now write a new program
                sem_wait(mutex);
                add_task(buffer, task, -1);  // Add own task
                sem_post(mutex);
                
                sem_post(sems[task]);  // Notify programmer
            } else {
                printf("Programmer %zu says programmer %d's program is INCORRECT\n", 
                       proger_num, task);
                
                // Send back to the same reviewer (this programmer)
                sem_wait(mutex);
                add_task(buffer, proger_num, task + NUM_PROGS);  // 3,4,5 means needs re-review
                sem_post(mutex);
                
                sem_post(sems[proger_num]);  // Notify myself
            }
        } 
        else if (task >= NUM_PROGS && task < 2*NUM_PROGS) {  // Need re-review (3,4,5)
            int original_proger = task - NUM_PROGS;
            printf("Programmer %zu is re-reviewing programmer %d's program\n", 
                   proger_num, original_proger);
            usleep(500000 + rand() % 300000);  // Simulate re-reviewing
            
            // Higher chance of being correct now (85%)
            int is_correct = (rand() % 100) < 85;
            
            if (is_correct) {
                printf("Programmer %zu says programmer %d's program is NOW CORRECT\n", 
                       proger_num, original_proger);
                
                // Programmer can now write a new program
                sem_wait(mutex);
                add_task(buffer, original_proger, -1);  // Add own task
                sem_post(mutex);
                
                sem_post(sems[original_proger]);  // Notify programmer
            } else {
                printf("Programmer %zu says programmer %d's program is STILL INCORRECT\n", 
                       proger_num, original_proger);
                
                // Send back to the same reviewer (this programmer) again
                sem_wait(mutex);
                add_task(buffer, proger_num, task);  // Same task number
                sem_post(mutex);
                
                sem_post(sems[proger_num]);  // Notify myself
            }
        }
    }
}

int main() {
    // unlink all shared memory
    sem_unlink(shared_mem_mutex);
    for (int i = 0; i < NUM_PROGS; i++) {
        sem_unlink(sem_names[i]);
    }
    shm_unlink(SHMEM_NAME);

    // opening semaphores
    mutex = sem_open(shared_mem_mutex, O_CREAT, 0644, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open(mutex)");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < NUM_PROGS; i++) {
        sems[i] = sem_open(sem_names[i], O_CREAT, 0644, 0);
        if (sems[i] == SEM_FAILED) {
            perror("sem_open(sem)");
            for (int j = 0; j < i; j++) {
                sem_close(sems[j]);
                sem_unlink(sem_names[j]);
            }
            sem_close(mutex);
            sem_unlink(shared_mem_mutex);
            
            exit(EXIT_FAILURE);
        }
    }

    // sigaction
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    int shmfd = shm_open(SHMEM_NAME, O_CREAT|O_RDWR, 0600);
    if (shmfd < 0) {
        perror("shm_open");
        return 1;
    }

    // Set the size of the shared memory segment
    if (ftruncate(shmfd, sizeof(shared_mem_t))) {
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

    // Initialize shared memory
    buffer->i = 0;
    buffer->j = 0;
    buffer->k = 0;
    memset(buffer->queue1, -1, sizeof(buffer->queue1));
    memset(buffer->queue2, -1, sizeof(buffer->queue2));
    memset(buffer->queue3, -1, sizeof(buffer->queue3));

    // Add initial tasks (each programmer starts by writing their own program)
    for (int i = 0; i < NUM_PROGS; i++) {
        add_task(buffer, i, -1);
        sem_post(sems[i]);
    }

    // Create child processes
    for (size_t i = 0; i < NUM_PROGS; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            run(i);
            exit(EXIT_SUCCESS);
        }
    }

    // Wait for all children to finish (though they won't in this case)
    while (wait(NULL) > 0);

    // Cleanup
    sigint_handler(0);

    return EXIT_SUCCESS;
}