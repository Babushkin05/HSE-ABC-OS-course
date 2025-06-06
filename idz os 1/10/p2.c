#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msg_buffer {
    long msg_type;
    char msg_text[128];
};

const int SEED = 65; 

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
    strcat(buffer, "\0");
    return buffer;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <key1> <key2>\n", argv[0]);
        exit(1);
    }

    char* readkey = argv[1];
    char* writekey = argv[2];

        key_t rkey = ftok(readkey, SEED);
    if (rkey == -1) {
        perror("ftok");
        exit(1);
    }
    key_t wkey = ftok(writekey, SEED);
    if (wkey == -1) {
        perror("ftok");
        exit(1);
    }

    int rmsgid = msgget(rkey, 0666 | IPC_CREAT);
    if (rmsgid == -1) {
        perror("msgget");
        exit(1);
    }
    int wmsgid = msgget(wkey, 0666 | IPC_CREAT);
    if (wmsgid == -1) {
        perror("msgget");
        exit(1);
    }

    const int CHUNC_SIZE = 128;
    size_t buffer_size = CHUNC_SIZE;
    size_t length = 0;
    char* data = malloc(buffer_size * sizeof(char));

    if (data == NULL) {
        perror("malloc");
        exit(1);
    }
    struct msg_buffer message;
    while (1)
    {
                printf("HERRR\n");
        ssize_t bytes_read = msgrcv(rmsgid, &message, sizeof(message.msg_text), 1, 0);
        
        if (bytes_read == -1) {
            perror("msgrcv");
            free(data);
            exit(1);
        } else if (bytes_read == 0) {
            if (length == 0) {
                free(data);
                exit(1);
            } else {
                break;
            }
        }
        strcpy(data + length, message.msg_text);

        length += CHUNC_SIZE;

        // there is '\0' in added string
        char* endline_pos = memchr(data + (length - bytes_read), '\0', bytes_read);
        if (endline_pos != NULL) {
            length = endline_pos - data + 1;
            break;
        }

        if (length >= buffer_size - 1) {
            buffer_size *= 2;
            data = realloc(data, buffer_size);
            if (data == NULL) {
                perror("realloc");
                exit(1);
            }
        }
    }
    data[length] = '\0';

    // process data
    ssize_t cnt = 0;
    int* arr = find_all_numbers(data, &cnt);
    if (arr == NULL) {
        perror("my func error");
        exit(1);
    }
    char* ans = make_string_from_array(arr, cnt);
    if (ans == NULL) {
        perror("my func error");
        free(arr);
        exit(1);
    }
    // write data to second channel
    size_t offset = 0;
    while (offset < length){
        size_t bytes_to_write = length - offset;
        if (bytes_to_write > CHUNC_SIZE) {
            bytes_to_write = CHUNC_SIZE;
        }

        struct msg_buffer message;
        message.msg_type = 1;
        strcpy(message.msg_text, ans + offset);
        ssize_t bytes_written = msgsnd(wmsgid, &message, sizeof(message.msg_text), 0);

        if (bytes_written == -1) {
            perror("msgsnd");
            exit(1);
        }

        offset += bytes_written;
    }

    free(arr);
    free(ans);

    return 0;
}