#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

int* find_all_numbers(char* string, ssize_t* ans_count) {
    ssize_t cap = 10;
    int* ans = (int*)malloc(cap * sizeof(int));
    if (ans == NULL) {
        perror("malloc");
        return NULL;
    }

    ssize_t strcap = 10;
    char* curstr = (char*)malloc(strcap * sizeof(char)); // string of readed number
    if (curstr == NULL) {
        perror("malloc");
        free(ans);
        return NULL;
    }

    curstr[0] = '\0';
    ssize_t curstr_len = 0; 
    *ans_count = 0;  

    for(ssize_t i = 0; string[i] != '\0'; ++i){
        if (string[i] >= '0' && string[i] <= '9'){
            if(curstr_len + 1 >= strcap) {
                strcap *= 2;
                char* new_curstr = (char*)realloc(curstr, strcap * sizeof(char));
                if (new_curstr == NULL) {
                    perror("realloc");
                    free(curstr);
                    free(ans);
                    return NULL;
                }
                curstr = new_curstr;
            }
            curstr[curstr_len] = string[i];
            curstr[++curstr_len] = '\0';
        }
        else {
            if(curstr_len > 0){
                int a = atoi(curstr);
                curstr_len = 0;
                if (*ans_count >= cap) {
                    cap *= 2;
                    int* new_ans = (int*)realloc(ans, cap * sizeof(int));
                    if (new_ans == NULL) {
                        perror("realloc");
                        free(curstr);
                        free(ans);
                        return NULL;
                    }
                    ans = new_ans;
                }
                ans[*ans_count] = a;
                (*ans_count)++;
            }
        }
    }

    if (curstr_len > 0) {
        int a = atoi(curstr);

        if (*ans_count >= cap) {
            cap *= 2;
            int* new_ans = (int*)realloc(ans, cap * sizeof(int));
            if (new_ans == NULL) {
                perror("realloc");
                free(curstr);
                free(ans);
                return NULL;
            }
            ans = new_ans;
        }

        ans[*ans_count] = a;
        (*ans_count)++;
    }

    free(curstr);
    return ans;
}

char* make_string_from_array(int* arr, ssize_t size) {
    const int MAX_NUMBER_LENGTH = 12; // 2^32 \sim 2 billoins 

    size_t buffer_size = 10 + (size * MAX_NUMBER_LENGTH) + (size * 2) + 1;
    char* buffer = (char*)malloc(buffer_size);
    if (buffer == NULL) {
        perror("malloc");
        return NULL;
    }

    sprintf(buffer, "there are %zu numbers: ", size);

    for (size_t i = 0; i < size; i++) {
        char number_str[MAX_NUMBER_LENGTH];
        sprintf(number_str, "%d", arr[i]); 
        strcat(buffer, number_str);       

        if (i < size - 1) {
            strcat(buffer, ", ");
        }
    }

    return buffer;
}

int main(int argc, char** argv) {
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <from> <to>\n", argv[0]);
        return 1;
    }

    char* from = argv[1];
    char* to = argv[2];

    int read_chan[2]; 
    if (pipe(read_chan) == -1) {
        perror("pipe");
        return 1;
    }

    int write_chan[2];
    if (pipe(write_chan) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid1 = fork();
    if(pid1 < 0){
        perror("fork");
        return 1;
    }
    else if(pid1 > 0){ // 1st process that read data from files

        int test_fd = open(from, O_RDONLY);
        if(test_fd < 0){
            perror("open");
            return 1;
        }
        char readbuf[5000];
        ssize_t bytes_read = read(test_fd, readbuf, sizeof(readbuf));
        if (bytes_read < 0) {
            perror("read");
            close(test_fd);
            return 1;
        }
        if (close(test_fd) < 0) {
            perror("close");
            return 1;
        }

        if (close(read_chan[0]) < 0) { // close channel for reading 
            perror("close");
            return 1;
        }
        if (write(read_chan[1], readbuf, bytes_read) < 0) {
            perror("write");
            close(read_chan[1]);
            return 1;
        }
        if (close(read_chan[1]) < 0) {
            perror("close");
            return 1;
        }

        wait(NULL); //  wait for 2 process

        // store data to file

        // read data from channel
        char writebuf[5000];
        if (close(write_chan[1]) < 0) {
            perror("close");
            return 1;
        }
        ssize_t bytes_read2 = read(write_chan[0], writebuf, sizeof(writebuf));
        if (bytes_read2 < 0) {
            perror("read");
            close(write_chan[0]);
            return 1;
        }
        if (close(write_chan[0]) < 0) {
            perror("close");
            return 1;
        }
        // write data to file
        int fd = creat(to, 0666);
        if (fd < 0) {
            perror("create");
            return 1;
        }
        if (write(fd, writebuf, bytes_read2) < 0) {
            perror("write");
            close(fd);
            return 1;
        }
        if (close(fd) < 0) {
            perror("close");
            return 1;
        }
    }
    else { // 2  process

        // handle information

        // read chan
        if (close(read_chan[1]) < 0) {
                perror("close");
                return 1;
            }
            char buf[5000];
            ssize_t bytes_read = read(read_chan[0], buf, sizeof(buf));
            if (bytes_read < 0) {
                perror("read");
                close(read_chan[0]);
                return 1;
            }
            if (close(read_chan[0]) < 0) {
                perror("close");
                return 1;
            }

            // process data
            ssize_t cnt = 0;
            int* data = find_all_numbers(buf, &cnt);
            if(data == NULL){
                perror("my func error");
                return 1;
            }
            char *ans = make_string_from_array(data, cnt);
            if(ans == NULL){
                perror("my func error");
                free(data);
                return 1;
            }

            if (close(write_chan[0]) < 0) {
                perror("close");
                free(data);
                free(ans);
                return 1;
            }

            // write data to second chanell
            if (write(write_chan[1], ans, strlen(ans)) < 0) {
                perror("write");
                close(write_chan[1]);
                free(data);
                free(ans);
                return 1;
            }
            if (close(write_chan[1]) < 0) {
                perror("close");
                free(data);
                free(ans);
                return 1;
            }

            free(data);
            free(ans);
    }

    return 0;
}