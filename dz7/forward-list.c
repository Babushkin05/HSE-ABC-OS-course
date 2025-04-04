#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

void rm_shmid(int shmid) {
    if (shmctl(shmid, IPC_RMID, NULL) < 0) {
        perror("shmctl(IPC_RMID)");
        exit(1);
    }
}

typedef struct{
    int data;
    ssize_t next;
} elem_t;

typedef struct 
{
    ssize_t head;
    elem_t elements[];
} forward_list;


int main(int argc, char const *argv[]) {
	key_t commonkey = ftok(".", 1);
    if (commonkey < 0) {
		perror("commonkey");
		return 1;
	}

    int mem_size = getpagesize();
	int shmid = shmget(commonkey, mem_size, IPC_CREAT | 0600);
	if (shmid < 0) {
		perror("shmget");
		return 1;
	}

    void *shmem = shmat(shmid, NULL, 0);
	if (!shmem) {
		perror("shmmem");
		return 1;
	}

    forward_list list = {.head=0};
    list.elements = shmem;

    
    // Mark to delete shmid after shmdt
    rm_shmid(shmid);

    // Do something useful with shmem
    *((char*)shmem) = 'A';

    if (shmdt(shmem) < 0) {
		perror("shmdt");
		return 1;
	}

    return 0;
}