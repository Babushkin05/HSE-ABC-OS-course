#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if(argc != 3){
        perror("program wait 2 filenames in flags: source and destination");
        return 1;
    }
    int fd1 = open(argv[1], O_RDONLY);
    
    if(fd1 < 0){
        perror("unable to open source file");
        return 1;
    }

    int fd2 = creat(argv[2], 0666);

    char buffer[1] = {};
    int res;
    while((res = read(fd1, buffer, sizeof(buffer))) != 0){
        if(res < 0){
            perror("reading error");
            return 1;
        }
        write(fd2, buffer, sizeof(buffer));
    }

    close(fd2);
    close(fd1);
    return 0;
}