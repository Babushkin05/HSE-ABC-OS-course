#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

// Удобная функция смэрти
void die(const char* msg) {
    perror(msg);
    exit(1);
}

// Статически выделенный стек для дочернего процесса
// (можно и через malloc, но зачем)
#define STACK_SIZE (1024 * 1024)
static char runc_stack[STACK_SIZE];

// Данные для дочернего процесса
struct params {
    const char* rootfs;
    char** cmd_argv;
};

// Функция, которая отмонтирует put_old
void umount_old_rootfs(const char* old_rootfs_path) {
    if (mount(NULL, old_rootfs_path, NULL, MS_REC | MS_PRIVATE, NULL))
        die("mount --make-rprivate old_rootfs");

    if (umount2(old_rootfs_path, MNT_DETACH))
        die("umount old_rootfs");
}

// Функция, с которой начнется выполнение дочернего (изолированного) процесса
int runc_cmd(void* arg) {
    struct params* param = arg;

    // TODO(ЭТАП 1):
    // 1. mount namespace
    // 2. mount --bind на rootfs
    // 3. pivot_root
    // 4. umount_old_rootfs put_old
    // 5. удалите put_old

    // TODO(ЭТАП 2):
    // 1. PID namespace
    // 2. mount procfs
    
    // Запуск программы
    char* cmd = param->cmd_argv[0];
    printf("============ %s ============\n", cmd);
    execv(cmd, param->cmd_argv);
    die("exec failed");
    return 1;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: isolate <rootfs> <cmd> [args...]\n");
        return 1;
    }

    struct params param;
    param.rootfs = argv[1];
    param.cmd_argv = &argv[2];

    // Запустить дочерний процесс
    int runc_pid = clone(runc_cmd, runc_stack + STACK_SIZE,
         SIGCHLD,
         &param);
    if (runc_pid < 0)
        die("clone");

    // Дождаться завершения дочернего процесса
    int wstatus;
    if (waitpid(runc_pid, &wstatus, 0) < 0)
        die("waitpid");
    printf("==== %s exited with %3d ====\n", argv[2], wstatus);
}