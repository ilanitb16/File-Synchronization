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

// provided function to handle the printing with random delays.
void write_message(const char *message, int count) {
    for (int i = 0; i < count; i++) {
        printf("%s\n", message);
        usleep((rand() % 100) * 1000); // Random delay between 0 and 99 milliseconds
    }
}

// Implement a simple locking mechanism using a lock file. Each process should create the lock file before writing
// and remove it after writing, ensuring mutual exclusion.

int acquire_lock() {
    // before a process starts writing, it opens the lock file. If it fails, the process will retry writing after 1ms
    int fd;
    while ((fd = open(LOCKFILE, O_CREAT | O_EXCL, 0644)) < 0) {
        usleep(1000); // Sleep for 1ms before trying again
    }
    return fd;
}

void release_lock(int fd) {
    close(fd);

    // delete lock file after writing in order to allow others to write to lock
    if (unlink(LOCKFILE) < 0) {
        perror("unlink failed.");
        exit(1);
    }
}

void write_to_file(const char *filename, const char *message, int count) {
  // lock the file so others don't have access.
    int fd = acquire_lock();

    // opening the file
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        perror("failed to open");
        release_lock(fd);
        exit(1);
    }

    // writing the message as many times as needed.

        write_message(message, count);
        if (fprintf(file, "%s\n", message) < 0) {
            perror("printf error");
            fclose(file);
            release_lock(fd);
            exit(1);
        }


    if (fclose(file) != 0) {
        perror("faild close the file");
        release_lock(fd);
        exit(1);
    }

    // releasing lock after finishing writing.
    release_lock(fd);
}

int main(int argc, char *argv[]) {
   // Ensure the program accepts the necessary arguments for the messages, the order of writing, and the count of writes.
   // input: number of messages, number of prints fror each process

    if (argc <= 4) {
        fprintf(stderr, "Usage: %s <message1> <message2> ... <count>", argv[0]) ;
        return 1;
    }

    int count = atoi(argv[argc - 1]); // number of times to print ,message
    int num_messages = argc - 2;

    const char *messages[num_messages]; // array to store messages

    // add messages to array
    for (int i = 0; i < num_messages; i++) {
        messages[i] = argv[i + 1];
    }

    // array to add the pid's
    pid_t pids[num_messages];

    for (int i = 0; i < num_messages; i++) {
        // parent forks process
        if ((pids[i] = fork()) < 0) {
            perror("fork");
            exit(1);
        }
        if (pids[i] == 0) {
            write_to_file(OUTPUTFILE, messages[i], count);
            exit(0);
        }
    }

    for (int i = 0; i < num_messages; i++) {
        // allows to wait for child processes to finish, using waitpid
        if (waitpid(pids[i], NULL, 0) == -1) {
            perror("waitpid");
            exit(1);
        }
    }

  //  printf("Finished writing to file: %s\n", OUTPUTFILE);
    return 0;
}
