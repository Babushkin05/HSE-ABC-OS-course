#include <errno.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <time.h>
#include <unistd.h>

#define NUM_PROGS 3
#define MAX_TASKS 50
#define CORRECT_PROBABILITY 70 // 70% chance program is correct

static _Atomic volatile sig_atomic_t unlink_done = 0;

// Структура для сообщений
typedef struct {
  long mtype; // тип сообщения (идентификатор программиста)
  int mtext;  // данные (номер задачи или -1 для новой программы)
} task_message;

// Идентификаторы семафоров и очередей сообщений
int task_semids[NUM_PROGS];
int msg_queues[NUM_PROGS];

// Структура для semctl
union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

void sigint_handler(int sig) {
  if (atomic_load(&unlink_done)) {
    return;
  }
  if (!atomic_load(&unlink_done)) {
    // Уничтожаем семафоры и очереди сообщений
    for (int i = 0; i < NUM_PROGS; i++) {
      semctl(task_semids[i], 0, IPC_RMID);
      msgctl(msg_queues[i], IPC_RMID, NULL);
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

void add_task(int msg_queue, size_t proger_num, int task) {
  task_message msg;
  msg.mtype = proger_num + 1; // +1 чтобы избежать 0
  msg.mtext = task;
  if (msgsnd(msg_queue, &msg, sizeof(msg.mtext), IPC_NOWAIT) == -1) {
    perror("msgsnd");
  }
}

int get_task(int msg_queue) {
  task_message msg;
  if (msgrcv(msg_queue, &msg, sizeof(msg.mtext), 0, IPC_NOWAIT) == -1) {
    if (errno != ENOMSG) {
      perror("msgrcv");
    }
    return -1;
  }
  return msg.mtext;
}

void run(size_t proger_num) {
  srand(time(NULL) + proger_num);

  while (1) {
    sem_wait(task_semids[proger_num]);

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
      sem_post(task_semids[reviewer]);
    } else if (task >= 0 && task < NUM_PROGS) {
      printf("Programmer %zu is reviewing programmer %d's program\n",
             proger_num, task);
      usleep(800000 + rand() % 600000);

      int is_correct = (rand() % 100) < CORRECT_PROBABILITY;

      if (is_correct) {
        printf("Programmer %zu says programmer %d's program is CORRECT\n",
               proger_num, task);

        add_task(msg_queues[task], task, -1);
        sem_post(task_semids[task]);
      } else {
        printf("Programmer %zu says programmer %d's program is INCORRECT\n",
               proger_num, task);

        add_task(msg_queues[proger_num], proger_num, task + NUM_PROGS);
        sem_post(task_semids[proger_num]);
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
        sem_post(task_semids[original_proger]);
      } else {
        printf(
            "Programmer %zu says programmer %d's program is STILL INCORRECT\n",
            proger_num, original_proger);

        add_task(msg_queues[proger_num], proger_num, task);
        sem_post(task_semids[proger_num]);
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

  if (proger_num == -1) {
    // Initializer process - sets up message queues and semaphores
    key_t key = ftok(".", 'S');
    if (key == -1) {
      perror("ftok");
      return EXIT_FAILURE;
    }

    // Create semaphores and message queues
    for (int i = 0; i < NUM_PROGS; i++) {
      task_semids[i] = semget(key + i + 1, 1, IPC_CREAT | 0600);
      if (task_semids[i] == -1) {
        perror("semget(task)");
        // Clean up already created semaphores
        for (int j = 0; j < i; j++) {
          semctl(task_semids[j], 0, IPC_RMID);
        }
        return EXIT_FAILURE;
      }

      union semun arg;
      arg.val = 0;
      if (semctl(task_semids[i], 0, SETVAL, arg) == -1) {
        perror("semctl(task)");
        // Clean up
        for (int j = 0; j <= i; j++) {
          semctl(task_semids[j], 0, IPC_RMID);
        }
        return EXIT_FAILURE;
      }

      // Create message queue for each programmer
      msg_queues[i] = msgget(key + NUM_PROGS + i + 1, IPC_CREAT | 0600);
      if (msg_queues[i] == -1) {
        perror("msgget");
        // Clean up
        for (int j = 0; j <= i; j++) {
          semctl(task_semids[j], 0, IPC_RMID);
          if (j < i)
            msgctl(msg_queues[j], IPC_RMID, NULL);
        }
        return EXIT_FAILURE;
      }
    }

    // Add initial tasks
    for (int i = 0; i < NUM_PROGS; i++) {
      add_task(msg_queues[i], i, -1);
      sem_post(task_semids[i]);
    }

    printf("Semaphores and message queues initialized. You can now run "
           "programmers.\n");
    pause(); // Wait for SIGINT
  } else {
    // Regular programmer process - get existing semaphores and message queues
    key_t key = ftok(".", 'S');
    if (key == -1) {
      perror("ftok");
      return EXIT_FAILURE;
    }

    for (int i = 0; i < NUM_PROGS; i++) {
      task_semids[i] = semget(key + i + 1, 1, 0600);
      if (task_semids[i] == -1) {
        perror("semget(task)");
        return EXIT_FAILURE;
      }

      msg_queues[i] = msgget(key + NUM_PROGS + i + 1, 0600);
      if (msg_queues[i] == -1) {
        perror("msgget");
        return EXIT_FAILURE;
      }
    }

    run(proger_num);
  }

  return EXIT_SUCCESS;
}