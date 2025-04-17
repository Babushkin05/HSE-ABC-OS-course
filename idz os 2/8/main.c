#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/sem.h>
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

// Структура для semop
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

// Идентификаторы семафоров System V
int mutex_semid;
int task_semids[NUM_PROGS];

void sigint_handler(int sig) {
    if (atomic_load(&unlink_done)) {
        return;  
    }

    if (!atomic_load(&unlink_done)) {
        // Уничтожаем семафоры System V
        semctl(mutex_semid, 0, IPC_RMID);
        for (int i = 0; i < NUM_PROGS; i++) {
            semctl(task_semids[i], 0, IPC_RMID);
        }

        atomic_store(&unlink_done, 1); 
    }

    kill(0, SIGINT);
    _exit(EXIT_SUCCESS); 
}

// Операции с семафорами System V
void sem_wait(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

void sem_post(int semid) {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
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
        sem_wait(task_semids[proger_num]);
        
        sem_wait(mutex_semid);
        int task = get_task(buffer, proger_num);
        sem_post(mutex_semid);
        
        if (task == -1) {
            printf("Programmer %zu is writing his own program\n", proger_num);
            usleep(500000 + rand() % 500000);
            
            size_t reviewer;
            do {
                reviewer = rand() % NUM_PROGS;
            } while (reviewer == proger_num);
            
            printf("Programmer %zu sends his program to programmer %zu for review\n", 
                   proger_num, reviewer);
            
            sem_wait(mutex_semid);
            add_task(buffer, reviewer, proger_num);
            sem_post(mutex_semid);
            
            sem_post(task_semids[reviewer]);
        } 
        else if (task >= 0 && task < NUM_PROGS) {
            printf("Programmer %zu is reviewing programmer %d's program\n", 
                   proger_num, task);
            usleep(800000 + rand() % 600000);
            
            int is_correct = (rand() % 100) < CORRECT_PROBABILITY;
            
            if (is_correct) {
                printf("Programmer %zu says programmer %d's program is CORRECT\n", 
                       proger_num, task);
                
                sem_wait(mutex_semid);
                add_task(buffer, task, -1);
                sem_post(mutex_semid);
                
                sem_post(task_semids[task]);
            } else {
                printf("Programmer %zu says programmer %d's program is INCORRECT\n", 
                       proger_num, task);
                
                sem_wait(mutex_semid);
                add_task(buffer, proger_num, task + NUM_PROGS);
                sem_post(mutex_semid);
                
                sem_post(task_semids[proger_num]);
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
                
                sem_wait(mutex_semid);
                add_task(buffer, original_proger, -1);
                sem_post(mutex_semid);
                
                sem_post(task_semids[original_proger]);
            } else {
                printf("Programmer %zu says programmer %d's program is STILL INCORRECT\n", 
                       proger_num, original_proger);
                
                sem_wait(mutex_semid);
                add_task(buffer, proger_num, task);
                sem_post(mutex_semid);
                
                sem_post(task_semids[proger_num]);
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

        // Create System V semaphores
        key_t key = ftok(".", 'S');
        if (key == -1) {
            perror("ftok");
            return EXIT_FAILURE;
        }

        // Create mutex semaphore
        mutex_semid = semget(key, 1, IPC_CREAT | 0600);
        if (mutex_semid == -1) {
            perror("semget(mutex)");
            return EXIT_FAILURE;
        }

        union semun arg;
        arg.val = 1;
        if (semctl(mutex_semid, 0, SETVAL, arg) == -1) {
            perror("semctl(mutex)");
            return EXIT_FAILURE;
        }

        // Create task semaphores
        for (int i = 0; i < NUM_PROGS; i++) {
            task_semids[i] = semget(key + i + 1, 1, IPC_CREAT | 0600);
            if (task_semids[i] == -1) {
                perror("semget(task)");
                // Clean up already created semaphores
                for (int j = 0; j < i; j++) {
                    semctl(task_semids[j], 0, IPC_RMID);
                }
                semctl(mutex_semid, 0, IPC_RMID);
                return EXIT_FAILURE;
            }

            arg.val = 0;
            if (semctl(task_semids[i], 0, SETVAL, arg) == -1) {
                perror("semctl(task)");
                // Clean up
                for (int j = 0; j <= i; j++) {
                    semctl(task_semids[j], 0, IPC_RMID);
                }
                semctl(mutex_semid, 0, IPC_RMID);
                return EXIT_FAILURE;
            }
        }

        // Add initial tasks
        for (int i = 0; i < NUM_PROGS; i++) {
            add_task(buffer, i, -1);
            sem_post(task_semids[i]);
        }

        printf("Shared memory and semaphores initialized. You can now run programmers.\n");
        pause(); // Wait for SIGINT
    } else {
        // Regular programmer process - get existing semaphores
        key_t key = ftok(".", 'S');
        if (key == -1) {
            perror("ftok");
            return EXIT_FAILURE;
        }

        mutex_semid = semget(key, 1, 0600);
        if (mutex_semid == -1) {
            perror("semget(mutex)");
            return EXIT_FAILURE;
        }

        for (int i = 0; i < NUM_PROGS; i++) {
            task_semids[i] = semget(key + i + 1, 1, 0600);
            if (task_semids[i] == -1) {
                perror("semget(task)");
                return EXIT_FAILURE;
            }
        }

        run(proger_num);
    }

    return EXIT_SUCCESS;
}