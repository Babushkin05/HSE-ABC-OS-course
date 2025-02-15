#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    char* file_name = argv[1];
    printf("%s", file_name);
    int fp = open(file_name, O_CREAT | O_TRUNC ,0666);
    if(fp < 0)
        printf("test");
    close(fp);
    return 0;
}