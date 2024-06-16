#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#define FAILED_EXIT = 1
#define SUCCESS_EXIT = 0

void write_to_file(const char *filename,const char *message, int count) {
    // open system call, append allowing to add to end of the file.
    int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
    // opening failed
    if (fd < 0) {
        perror("opening the file has failed.");
        exit(1);
    }

    size_t msg_len = strlen(message);
    char *msg_with_newline = malloc(msg_len + 2);
    if (!msg_with_newline) {
        perror("malloc");
        close(fd);
        exit(1);
    }

    // Copy the message and append a newline character
    strcpy(msg_with_newline, message);
    msg_with_newline[msg_len] = '\n';
    msg_with_newline[msg_len + 1] = '\0';

    for (int i = 0; i < count; i++) {
        if (write(fd, msg_with_newline, msg_len + 1) < 0) {
            perror("failed to write to file");
            free(msg_with_newline);
            close(fd);
            exit(1);
        }
    }

    free(msg_with_newline);

//    for (int i = 0; i < count; i++) {
//       //  write the message to the file.
//        if (write(fd, message, strlen(message)) < 0) {
//            perror("failed to write to file");
//            close(fd);
//            exit(1);
//        }
//    }

    // close the file descriptor.
    if (close(fd) < 0) {
        perror("close");
        exit(1);
    }

}

//void child_process(const char *filename, const char *process_name) {
//    write_to_file(process_name,"hi", filename);
//    exit(0);
//}

int main(int argc, char *argv[]) {
    // The program should accept input params to specify the messages and number of times each process writes to file.
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <parent_message> <child1_message> <child2_message> <count>\n", argv[0]);
        return 1;
    }
    // get messages from args.
    const char *parent_message = argv[1];
    const char *child1_message = argv[2];
    const char *child2_message = argv[3];

    // convert string value to int.
    int count = atoi(argv[4]);

    const char *filename = "output.txt"; // the file which the output will be written to.
    pid_t pid1, pid2; // id of the child processes

    // first fork
    pid1 = fork();
    // fork failed
    if (pid1 < 0) {
        perror("first fork has failed.");
        exit(1);
    }
    // this is the first process.
    if (pid1 == 0) {
        write_to_file(filename, child1_message,count);
        exit(0);
    }

    // make sure child 1 finishes writing first.
    waitpid(pid1, NULL, 0);

    // second fork
    pid2 = fork();
    // fork failed
    if (pid2 < 0) {
        perror("second fork has failed.");
        exit(1);
    }
    // this is the second process.
    if (pid2 == 0) {
        write_to_file(filename, child2_message,count);
        exit(0);
    }

    // wait for both to finish
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    // Parent writes to the file
    write_to_file(filename, parent_message, count);

    // finishing statement
    printf("Finished writing to file: %s\n", filename);

    return 0;
}
