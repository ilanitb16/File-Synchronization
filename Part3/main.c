#include "buffered_open.h"
#include <stdio.h>
#include <string.h>

int main() {
    buffered_file_t *bf = buffered_open("example.txt", O_WRONLY | O_CREAT | O_PREAPPEND, 0644);
    if (!bf) {
        perror("buffered_open");
        return 1;
    }

    const char *text = "Hello, World!";
    if (buffered_write(bf, text, strlen(text)) == -1) {
        perror("buffered_write");
        buffered_close(bf);
        return 1;
    }

    if (buffered_close(bf) == -1) {
        perror("buffered_close");
        return 1;
    }

    buffered_file_t *bf_read = buffered_open("example.txt", O_RDONLY, 0644);
    if (!bf_read) {
        perror("buffered_open");
        return 1;
    }

    char read_buf[128];
    ssize_t bytes_read = buffered_read(bf_read, read_buf, sizeof(read_buf) - 1);
    if (bytes_read == -1) {
        perror("buffered_read");
        buffered_close(bf_read);
        return 1;
    }

    read_buf[bytes_read] = '\0';
    printf("Read from file:\n %s\n", read_buf);

    if (buffered_close(bf_read) == -1) {
        perror("buffered_close");
        return 1;
    }

    return 0;
}
