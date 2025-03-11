#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <from> <to> <chan1> <chan2>\n", argv[0]);
        exit(1);
    }

    char* from = argv[1];
    char* to = argv[2];
    char* readchan = argv[3];
    char* writechan = argv[4];

    if (mkfifo(readchan, 0666) == -1) {
        perror("mkfifo");
        exit(1);
    }

    if (mkfifo(writechan, 0666) == -1) {
        perror("mkfifo");
        exit(1);
    }

    // reading from file
    int test_fd = open(from, O_RDONLY);
    if (test_fd < 0) {
        perror("open");
        exit(1);
    }
    char readbuf[5000];
    ssize_t bytes_read = read(test_fd, readbuf, sizeof(readbuf));
    if (bytes_read < 0) {
        perror("read");
        close(test_fd);
        exit(1);
    }
    if (close(test_fd) < 0) {
        perror("close");
        exit(1);
    }

    // writing to channel
    int read_fd = open(readchan, O_WRONLY);
    if (read_fd < 0) {
        perror("open");
        exit(1);
    }
    if (write(read_fd, readbuf, bytes_read) < 0) {
        perror("write");
        close(read_fd);
        exit(1);
    }
    if (close(read_fd) < 0) {
        perror("close");
        exit(1);
    }

    // read data from channel
    char writebuf[5000];

    int write_fd = open(writechan, O_RDONLY);
    if (write < 0) {
        perror("open");
        exit(1);
    }

    ssize_t bytes_read2 = read(write_fd, writebuf, sizeof(writebuf));
    if (bytes_read2 < 0) {
        perror("read");
        close(write_fd);
        exit(1);
    }
    if (close(write_fd) < 0) {
        perror("close");
        exit(1);
    }

    // write data to file
    int fd = creat(to, 0666);
    if (fd < 0) {
        perror("create");
        exit(1);
    }
    if (write(fd, writebuf, bytes_read2) < 0) {
        perror("write");
        close(fd);
        exit(1);
    }
    if (close(fd) < 0) {
        perror("close");
        exit(1);
    }

    if (unlink(readchan) == -1) {
        perror("unlink");
        exit(1);
    }

    if (unlink(writechan) == -1) {
        perror("unlink");
        exit(1);
    }

    return 0;
}