#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <stdatomic.h>
#include <sys/stat.h>

#define SHM_NAME "/counter_shm"

struct shared_data {
    _Atomic int counter;
    _Atomic int process_count;
};

static volatile sig_atomic_t keep_running = 1;
static struct shared_data *shared_mem;
static int shm_fd;

void handle_signal(int sig) {
    (void)sig;
    keep_running = 0;
}

void cleanup() {
    if (shared_mem) {
        int remaining = atomic_fetch_sub(&shared_mem->process_count, 1) - 1;
        
        munmap(shared_mem, sizeof(struct shared_data));
        
        if (remaining == 0) {
            shm_unlink(SHM_NAME);
        } 

        close(shm_fd);
    }
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    struct stat st;
    if (fstat(shm_fd, &st) == -1) {
        perror("fstat");
        return 1;
    }

    if (st.st_size == 0) {
        if (ftruncate(shm_fd, sizeof(struct shared_data)) == -1) {
            perror("ftruncate failed");
            return 1;
        }
    }

    shared_mem = mmap(NULL, sizeof(struct shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    if (st.st_size == 0) {
        atomic_store(&shared_mem->counter, 0);
        atomic_store(&shared_mem->process_count, 0);
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