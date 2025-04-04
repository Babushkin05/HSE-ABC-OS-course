#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <stdatomic.h>

struct shared_data {
    _Atomic int counter;
    _Atomic int process_count;
};

static volatile sig_atomic_t keep_running = 1;
static int shm_id;
static struct shared_data *shared_mem;

void handle_signal(int sig) {
    (void)sig;
    keep_running = 0;
}

void cleanup() {
    if (shared_mem) {
        int remaining = atomic_fetch_sub(&shared_mem->process_count, 1) - 1;
        
        if (remaining == 0) {
            shmdt(shared_mem);
            shmctl(shm_id, IPC_RMID, NULL);
        } else {
            shmdt(shared_mem);
        }
    }
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    key_t key = ftok(".", 'S');
    shm_id = shmget(key, sizeof(struct shared_data), 0666 | IPC_CREAT);
    
    if (shm_id == -1) {
        perror("shmget");
        return 1;
    }

    shared_mem = (struct shared_data *)shmat(shm_id, NULL, 0);
    if (shared_mem == (void *)-1) {
        perror("shmat");
        return 1;
    }

    atomic_fetch_add(&shared_mem->process_count, 1);

    while (keep_running) {
        int old_value = atomic_fetch_add(&shared_mem->counter, 1);
        printf("%d\n", old_value);
        fflush(stdout);
        usleep(400000); // 400 ms
    }

    cleanup();
    return 0;
}