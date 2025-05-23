#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int main() {
    FILE* file = fopen("test", "w");
    if (file == NULL) {
        perror("fopen");
        return 1;
    }
    fclose(file);

    int depth = 0;
    char prev_link[32] = "test";
    char new_link[32];

    while (1) {
        snprintf(new_link, sizeof(new_link), "test%d", depth + 1);
        
        if (symlink(prev_link, new_link) != 0) {
            perror("symlink");
            return 1;
        }

        int fd = open(new_link, O_RDONLY);
        if (fd == -1) {
            if (errno == ELOOP) {
                printf("recursion limit: %d\n", depth);
                break;
            } else {
                perror("open");
                return 1;
            }
        } else {
            close(fd);
        }

        strcpy(prev_link, new_link);
        depth++;
    }

    // type rm test* to delete symlimks
    return 0;
}