#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define LOCKFILE "lockfile.lock"
#define OUTPUTFILE "output2.txt"

// function to handle the printing with random delays.
void write_message(const char *message, int count) {
    for (int i = 0; i < count; i++) {
        printf("%s", message);
        usleep((rand() % 100) * 1000); // Random delay between 0 and 99 milliseconds
    }
}

// Implement a simple locking mechanism using a lock file. Each process should create the lock file before writing
// and remove it after writing, ensuring mutual exclusion.

void acquire_lock() {
    int fd;
    while ((fd = open(LOCKFILE, O_CREAT | O_EXCL, 0644)) < 0) {
        usleep(1000); // Sleep for 1ms before trying again
    }
    close(fd);
}

void release_lock() {
    // delete file
    if (unlink(LOCKFILE) < 0) {
        perror("unlink failed.");
        exit(1);
    }
}

void write_to_file(const char *filename, const char *message, int count) {
    acquire_lock();

    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        perror("fopen");
        release_lock();
        exit(1);
    }

    for (int i = 0; i < count; i++) {
        if (fprintf(file, "%s\n", message) < 0) {
            perror("fprintf");
            fclose(file);
            release_lock();
            exit(1);
        }
    }

    if (fclose(file) != 0) {
        perror("fclose");
        release_lock();
        exit(1);
    }

    release_lock();
}

int main(int argc, char *argv[]) {
   // Ensure the program accepts the necessary arguments for the messages, the order of writing, and the count of writes.
    if (argc < 6) {
        fprintf(stderr, "Usage: %s <message1> <message2> ... <order> <count>", argv[0]) ;
        return 1;
    }

    int count = atoi(argv[argc - 1]);
    int num_messages = argc - 2;
    const char *messages[num_messages];

    for (int i = 0; i < num_messages; i++) {
        messages[i] = argv[i + 1];
    }

    pid_t pids[num_messages];

    for (int i = 0; i < num_messages; i++) {
        if ((pids[i] = fork()) < 0) {
            perror("fork");
            exit(1);
        } else if (pids[i] == 0) {
            write_to_file(OUTPUTFILE, messages[i], count);
            exit(0);
        }
    }

    for (int i = 0; i < num_messages; i++) {
        if (waitpid(pids[i], NULL, 0) == -1) {
            perror("waitpid");
            exit(1);
        }
    }

    printf("Finished writing to file: %s\n", OUTPUTFILE);
    return 0;
}
