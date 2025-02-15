#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    char* file_name = argv[1];
    int fd = open(file_name,  O_CREAT ,0666);
    if(fd < 0)
        printf("err");
    char string[10];
    int res = read(fd, string, 10);
    printf("%.*s", res, string);
    close(fd);
    return 0;
}