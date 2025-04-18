#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#define NUM_PROGS 3
#define MAX_TASKS 50
#define CORRECT_PROBABILITY 70 // 70% chance program is correct
#define QUEUE_NAME_SIZE 20

static _Atomic volatile sig_atomic_t unlink_done = 0;

typedef struct {
    int mtext;  
} task_message;

sem_t *task_semaphores[NUM_PROGS];
mqd_t msg_queues[NUM_PROGS];
char queue_names[NUM_PROGS][QUEUE_NAME_SIZE];

void sigint_handler(int sig) {
    if (atomic_load(&unlink_done)) {
        return;
    }
    if (!atomic_load(&unlink_done)) {
        for (int i = 0; i < NUM_PROGS; i++) {
            sem_close(task_semaphores[i]);
            char sem_name[QUEUE_NAME_SIZE];
            snprintf(sem_name, QUEUE_NAME_SIZE, "/prog_sem_%d", i);
            sem_unlink(sem_name);
            
            mq_close(msg_queues[i]);
            mq_unlink(queue_names[i]);
        }
        atomic_store(&unlink_done, 1);
    }

    kill(0, SIGINT);
    _exit(EXIT_SUCCESS);
}

void add_task(mqd_t msg_queue, size_t proger_num, int task) {
    task_message msg;
    msg.mtext = task;
    if (mq_send(msg_queue, (const char *)&msg, sizeof(msg), 0) == -1) {
        perror("mq_send");
    }
}

int get_task(mqd_t msg_queue) {
    task_message msg;
    unsigned int prio;
    if (mq_receive(msg_queue, (char *)&msg, sizeof(msg), &prio) == -1) {
        if (errno != EAGAIN) {
            perror("mq_receive");
        }
        return -1;
    }
    return msg.mtext;
}

void run(size_t proger_num) {
    srand(time(NULL) + proger_num);

    while (1) {
        sem_wait(task_semaphores[proger_num]);

        int task = get_task(msg_queues[proger_num]);

        if (task == -1) {
            printf("Programmer %zu is writing his own program\n", proger_num);
            usleep(500000 + rand() % 500000);

            size_t reviewer;
            do {
                reviewer = rand() % NUM_PROGS;
            } while (reviewer == proger_num);

            printf("Programmer %zu sends his program to programmer %zu for review\n",
                   proger_num, reviewer);

            add_task(msg_queues[reviewer], proger_num, proger_num);
            sem_post(task_semaphores[reviewer]);
        } else if (task >= 0 && task < NUM_PROGS) {
            printf("Programmer %zu is reviewing programmer %d's program\n",
                   proger_num, task);
            usleep(800000 + rand() % 600000);

            int is_correct = (rand() % 100) < CORRECT_PROBABILITY;

            if (is_correct) {
                printf("Programmer %zu says programmer %d's program is CORRECT\n",
                       proger_num, task);

                add_task(msg_queues[task], task, -1);
                sem_post(task_semaphores[task]);
            } else {
                printf("Programmer %zu says programmer %d's program is INCORRECT\n",
                       proger_num, task);

                add_task(msg_queues[proger_num], proger_num, task + NUM_PROGS);
                sem_post(task_semaphores[proger_num]);
            }
        } else if (task >= NUM_PROGS && task < 2 * NUM_PROGS) {
            int original_proger = task - NUM_PROGS;
            printf("Programmer %zu is re-reviewing programmer %d's program\n",
                   proger_num, original_proger);
            usleep(500000 + rand() % 300000);

            int is_correct = (rand() % 100) < 85;

            if (is_correct) {
                printf("Programmer %zu says programmer %d's program is NOW CORRECT\n",
                       proger_num, original_proger);

                add_task(msg_queues[original_proger], original_proger, -1);
                sem_post(task_semaphores[original_proger]);
            } else {
                printf("Programmer %zu says programmer %d's program is STILL INCORRECT\n",
                       proger_num, original_proger);

                add_task(msg_queues[proger_num], proger_num, task);
                sem_post(task_semaphores[proger_num]);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <programmer_id>\n", argv[0]);
        fprintf(stderr, " programmer_id can be 0, 1, 2 or -1 for initializer\n");
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

    // Generate queue names
    for (int i = 0; i < NUM_PROGS; i++) {
        snprintf(queue_names[i], QUEUE_NAME_SIZE, "/prog_queue_%d", i);
    }

    if (proger_num == -1) {
        // Initializer process - sets up message queues and semaphores
        // Create semaphores and message queues
        for (int i = 0; i < NUM_PROGS; i++) {
            char sem_name[QUEUE_NAME_SIZE];
            snprintf(sem_name, QUEUE_NAME_SIZE, "/prog_sem_%d", i);
            
            sem_unlink(sem_name);
            task_semaphores[i] = sem_open(sem_name, O_CREAT | O_EXCL, 0600, 0);
            if (task_semaphores[i] == SEM_FAILED) {
                perror("sem_open");
                // Clean up already created semaphores
                for (int j = 0; j < i; j++) {
                    sem_close(task_semaphores[j]);
                    snprintf(sem_name, QUEUE_NAME_SIZE, "/prog_sem_%d", j);
                    sem_unlink(sem_name);
                }
                return EXIT_FAILURE;
            }

            // Create message queue for each programmer
            mq_unlink(queue_names[i]);
            struct mq_attr attr = {
                .mq_flags = 0,
                .mq_maxmsg = 10,
                .mq_msgsize = sizeof(task_message)
            };
            msg_queues[i] = mq_open(queue_names[i], O_CREAT | O_EXCL | O_RDWR, 0600, &attr);
            if (msg_queues[i] == (mqd_t)-1) {
                perror("mq_open");
                // Clean up
                for (int j = 0; j <= i; j++) {
                    sem_close(task_semaphores[j]);
                    snprintf(sem_name, QUEUE_NAME_SIZE, "/prog_sem_%d", j);
                    sem_unlink(sem_name);
                    if (j < i) {
                        mq_close(msg_queues[j]);
                        mq_unlink(queue_names[j]);
                    }
                }
                return EXIT_FAILURE;
            }
        }

        // Add initial tasks
        for (int i = 0; i < NUM_PROGS; i++) {
            add_task(msg_queues[i], i, -1);
            sem_post(task_semaphores[i]);
        }

        printf("Semaphores and message queues initialized. You can now run "
               "programmers.\n");
        pause(); // Wait for SIGINT
    } else {
        // Regular programmer process - get existing semaphores and message queues
        for (int i = 0; i < NUM_PROGS; i++) {
            char sem_name[QUEUE_NAME_SIZE];
            snprintf(sem_name, QUEUE_NAME_SIZE, "/prog_sem_%d", i);
            
            task_semaphores[i] = sem_open(sem_name, 0);
            if (task_semaphores[i] == SEM_FAILED) {
                perror("sem_open");
                return EXIT_FAILURE;
            }

            msg_queues[i] = mq_open(queue_names[i], O_RDWR);
            if (msg_queues[i] == (mqd_t)-1) {
                perror("mq_open");
                return EXIT_FAILURE;
            }
        }

        run(proger_num);
    }

    return EXIT_SUCCESS;
}