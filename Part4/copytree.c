#include "copytree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

void copy_file(const char *src, const char *dest, int copy_symlinks, int copy_permissions) {
    struct stat stat_buf;

    // Check if src is a symbolic link
    if (lstat(src, &stat_buf) < 0) {
        perror("lstat failed");
        return;
    }

    if (S_ISLNK(stat_buf.st_mode) && copy_symlinks) {
        // Copy the symbolic link itself
        char symlink_target[PATH_MAX];
        ssize_t len = readlink(src, symlink_target, sizeof(symlink_target) - 1);
        if (len < 0) {
            perror("readlink failed");
            return;
        }
        symlink_target[len] = '\0';

        if (symlink(symlink_target, dest) < 0) {
            perror("symlink failed");
            return;
        }
    } else {
        // Copy the file content
        int src_fd = open(src, O_RDONLY);
        if (src_fd < 0) {
            perror("open src failed");
            return;
        }

        int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, stat_buf.st_mode & 0777);
        if (dest_fd < 0) {
            perror("open dest failed");
            close(src_fd);
            return;
        }

        char buffer[4096];
        ssize_t bytes;
        while ((bytes = read(src_fd, buffer, sizeof(buffer))) > 0) {
            if (write(dest_fd, buffer, bytes) != bytes) {
                perror("write failed");
                close(src_fd);
                close(dest_fd);
                return;
            }
        }

        if (bytes < 0) {
            perror("read failed");
        }

        close(src_fd);
        close(dest_fd);
    }

    if (copy_permissions && !S_ISLNK(stat_buf.st_mode)) {
        // Copy file permissions
        if (chmod(dest, stat_buf.st_mode) < 0) {
          //  printf("%s \n", dest);
            perror("chmod failed");
        }
    }
}

void create_directory_recursive(const char *dir) {
    char tmp[PATH_MAX];
    snprintf(tmp, sizeof(tmp), "%s", dir);

    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, S_IRWXU) < 0 && errno != EEXIST) {
                perror("mkdir failed");
                return;
            }
            *p = '/';
        }
    }

    if (mkdir(tmp, S_IRWXU) < 0 && errno != EEXIST) {
        perror("mkdir failed");
    }
}

void copy_directory(const char *src, const char *dest, int copy_symlinks, int copy_permissions) {
    DIR *dir = opendir(src);
    if (!dir) {
        perror("opendir failed");
        return;
    }

    struct dirent *entry;
    create_directory_recursive(dest);

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char src_path[PATH_MAX];
        char dest_path[PATH_MAX];
        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, entry->d_name);

        struct stat stat_buf;
        if (lstat(src_path, &stat_buf) < 0) {
            perror("lstat failed");
            continue;
        }

        if (S_ISDIR(stat_buf.st_mode)) {
            copy_directory(src_path, dest_path, copy_symlinks, copy_permissions);
        } else {
            copy_file(src_path, dest_path, copy_symlinks, copy_permissions);
        }
    }

    closedir(dir);
}
