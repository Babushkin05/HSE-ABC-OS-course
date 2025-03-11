#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int* find_all_numbers(char* string, ssize_t* ans_count) {
    ssize_t cap = 10;
    int* ans = (int*)malloc(cap * sizeof(int));
    if (ans == NULL) {
        perror("malloc");
        return NULL;
    }

    ssize_t strcap = 10;
    char* curstr =
        (char*)malloc(strcap * sizeof(char));  // string of readed number
    if (curstr == NULL) {
        perror("malloc");
        free(ans);
        return NULL;
    }

    curstr[0] = '\0';
    ssize_t curstr_len = 0;
    *ans_count = 0;

    for (ssize_t i = 0; string[i] != '\0'; ++i) {
        if (string[i] >= '0' && string[i] <= '9') {
            if (curstr_len + 1 >= strcap) {
                strcap *= 2;
                char* new_curstr =
                    (char*)realloc(curstr, strcap * sizeof(char));
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
        } else {
            if (curstr_len > 0) {
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
    const int MAX_NUMBER_LENGTH = 12;  // 2^32 \sim 2 billoins

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
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <chan1> <chan2>\n", argv[0]);
        exit(1);
    }

    char* readchan = argv[1];
    char* writechan = argv[2];

    // reading from channel
    int read_fd = open(readchan, O_RDONLY);
    if (read_fd < 0) {
        perror("open");
        exit(1);
    }
    char buf[5000];
    ssize_t bytes_read = read(read_fd, buf, sizeof(buf));
    if (bytes_read < 0) {
        perror("read");
        close(read_fd);
        exit(1);
    }
    if (close(read_fd) < 0) {
        perror("close");
        exit(1);
    }

    // process data
    ssize_t cnt = 0;
    int* data = find_all_numbers(buf, &cnt);
    if (data == NULL) {
        perror("my func error");
        exit(1);
    }
    char* ans = make_string_from_array(data, cnt);
    if (ans == NULL) {
        perror("my func error");
        free(data);
        exit(1);
    }

    // write data to second channel
    int write_fd = open(writechan, O_WRONLY);
    if (write_fd < 0) {
        perror("open");
        free(data);
        free(ans);
        exit(1);
    }

    if (write(write_fd, ans, strlen(ans)) < 0) {
        perror("write");
        close(write_fd);
        free(data);
        free(ans);
        exit(1);
    }

    if (close(write_fd) < 0) {
        perror("close");
        free(data);
        free(ans);
        exit(1);
    }

    free(data);
    free(ans);
    
    return 0;
}