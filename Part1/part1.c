#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

void write_to_file(const char *process_name, const char *filename) {
    // open system call, append allowing to add to end of the file.
    int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
    // opening failed
    if (fd < 0) {
        perror("opening the file has failed.");
        exit(EXIT_FAILURE);
    }
}

void child_process(const char *filename, const char *process_name) {
    write_to_file(process_name, filename);
    exit(EXIT_SUCCESS);
}

int main() {
    const char *filename = "result.txt"; // the file which the output will be written to.
    pid_t pid1, pid2; // id of the child processes

    // first fork
    pid1 = fork();
    // fork failed
    if (pid1 < 0) {
        perror("first fork has failed.");
        exit(EXIT_FAILURE);
    }
    // this is the first process.
    if (pid1 == 0) {
        child_process(filename, "First Child Process");
    }

    // second fork
    pid2 = fork();
    // fork failed
    if (pid2 < 0) {
        perror("second fork has failed.");
        exit(EXIT_FAILURE);
    }
    // this is the second process.
    if (pid2 == 0) {
        child_process(filename, "Second Child Process");
    }

    // Write message from parent
    write_to_file("Parent Process", filename);

    // wait for both to finish
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    // finishing statement
    printf("Finished writing to file: %s\n", filename);

    return 0;
}