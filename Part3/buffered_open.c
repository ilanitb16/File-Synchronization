#include "buffered_open.h"
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

buffered_file_t *buffered_open(const char *pathname, int flags, ...) {
    buffered_file_t *bf = (buffered_file_t *)malloc(sizeof(buffered_file_t));
    if (!bf) {
        errno = ENOMEM;
        return NULL;
    }

    va_list args;
    va_start(args, flags);
    mode_t mode = 0;
    if (flags & O_CREAT) {
        mode = va_arg(args, int);
    }
    va_end(args);

    // Remove O_PREAPPEND from the flags before opening the file
    bf->preappend = flags & O_PREAPPEND;
    flags &= ~O_PREAPPEND;

    bf->fd = open(pathname, flags, mode);
    if (bf->fd == -1) {
        free(bf);
        return NULL;
    }

    bf->read_buffer = (char *)malloc(BUFFER_SIZE);
    bf->write_buffer = (char *)malloc(BUFFER_SIZE);
    if (!bf->read_buffer || !bf->write_buffer) {
        close(bf->fd);
        free(bf->read_buffer);
        free(bf->write_buffer);
        free(bf);
        errno = ENOMEM;
        return NULL;
    }

    bf->read_buffer_size = BUFFER_SIZE;
    bf->write_buffer_size = BUFFER_SIZE;
    bf->read_buffer_pos = 0;
    bf->write_buffer_pos = 0;
    bf->flags = flags;

    return bf;
}

ssize_t buffered_write(buffered_file_t *bf, const void *buf, size_t count) {
    const char *data = (const char *)buf;
    size_t total_written = 0;

    while (count > 0) {
        size_t space_left = bf->write_buffer_size - bf->write_buffer_pos;
        size_t to_write = count < space_left ? count : space_left;

        memcpy(bf->write_buffer + bf->write_buffer_pos, data, to_write);
        bf->write_buffer_pos += to_write;
        data += to_write;
        count -= to_write;
        total_written += to_write;

        if (bf->write_buffer_pos == bf->write_buffer_size) {
            if (buffered_flush(bf) == -1) {
                return -1;
            }
        }
    }

    return total_written;
}

//int buffered_flush(buffered_file_t *bf) {
//    if (bf->write_buffer_pos == 0) {
//        return 0;
//    }
//
//    ssize_t written = 0;
//    if (bf->preappend) {
//        // Read entire existing content
//        off_t current_offset = lseek(bf->fd, 0, SEEK_CUR);
//        lseek(bf->fd, 0, SEEK_SET);
//        char *temp_buffer = NULL;
//        ssize_t existing_size = 0;
//        ssize_t read_size = 0;
//
//        while ((read_size = read(bf->fd, bf->read_buffer, BUFFER_SIZE)) > 0) {
//            temp_buffer = (char *)realloc(temp_buffer, existing_size + read_size);
//            memcpy(temp_buffer + existing_size, bf->read_buffer, read_size);
//            existing_size += read_size;
//        }
//
//        lseek(bf->fd, 0, SEEK_SET);
//
//        // Write new content
//        written = write(bf->fd, bf->write_buffer, bf->write_buffer_pos);
//        if (written == -1) {
//            free(temp_buffer);
//            return -1;
//        }
//
//        // Append existing content
//        if (existing_size > 0) {
//            written = write(bf->fd, temp_buffer, existing_size);
//            if (written == -1) {
//                free(temp_buffer);
//                return -1;
//            }
//        }
//
//        free(temp_buffer);
//        lseek(bf->fd, current_offset, SEEK_SET);
//    } else {
//        written = write(bf->fd, bf->write_buffer, bf->write_buffer_pos);
//        if (written == -1) {
//            return -1;
//        }
//    }
//
//    bf->write_buffer_pos = 0;
//    return 0;
//}

int buffered_flush(buffered_file_t *bf) {
    if (bf->write_buffer_pos == 0) {
        return 0;
    }

    ssize_t written = 0;
    if (bf->preappend) {
        // Create a temporary file
        char temp_path[] = "/tmp/buffered_write_temp.XXXXXX";
        int temp_fd = mkstemp(temp_path);
        if (temp_fd == -1) {
            return -1;
        }

        // Read existing content from the current position to the end of the file into the temporary file
        off_t current_offset = lseek(bf->fd, 0, SEEK_CUR);
        off_t original_offset = current_offset;
        ssize_t read_size;
        while ((read_size = read(bf->fd, bf->read_buffer, bf->read_buffer_size)) > 0) {
            if (write(temp_fd, bf->read_buffer, read_size) != read_size) {
                close(temp_fd);
                unlink(temp_path);
                return -1;
            }
        }
        lseek(bf->fd, original_offset, SEEK_SET);

        // Write buffered data to the original file
        written = write(bf->fd, bf->write_buffer, bf->write_buffer_pos);
        if (written == -1) {
            close(temp_fd);
            unlink(temp_path);
            return -1;
        }

        // Append the backed-up data from the temporary file back to the original file
        lseek(temp_fd, 0, SEEK_SET);
        while ((read_size = read(temp_fd, bf->read_buffer, bf->read_buffer_size)) > 0) {
            if (write(bf->fd, bf->read_buffer, read_size) != read_size) {
                close(temp_fd);
                unlink(temp_path);
                return -1;
            }
        }

        // Clean up temporary file and adjust file offset
        close(temp_fd);
        unlink(temp_path);
        lseek(bf->fd, current_offset + bf->write_buffer_pos, SEEK_SET);
    } else {
        written = write(bf->fd, bf->write_buffer, bf->write_buffer_pos);
        if (written == -1) {
            return -1;
        }
    }

    bf->write_buffer_pos = 0;
    return 0;
}

ssize_t buffered_read(buffered_file_t *bf, void *buf, size_t count) {
    size_t remaining = count;
    char *buf_ptr = (char *)buf;

    while (remaining > 0) {
        if (bf->read_buffer_pos == 0) {
            ssize_t read_bytes = read(bf->fd, bf->read_buffer, bf->read_buffer_size);
            if (read_bytes == -1) {
                return -1;
            }
            if (read_bytes == 0) {
                break; // End of file
            }
            bf->read_buffer_size = read_bytes;
        }

        size_t to_read = (remaining < bf->read_buffer_size - bf->read_buffer_pos) ? remaining : bf->read_buffer_size - bf->read_buffer_pos;
        memcpy(buf_ptr, bf->read_buffer + bf->read_buffer_pos, to_read);
        bf->read_buffer_pos += to_read;
        buf_ptr += to_read;
        remaining -= to_read;

        if (bf->read_buffer_pos == bf->read_buffer_size) {
            bf->read_buffer_pos = 0; // Reset read buffer position
        }
    }

    return count - remaining;
}

int buffered_close(buffered_file_t *bf) {
    if (buffered_flush(bf) == -1) {
        return -1;
    }

    int result = close(bf->fd);
    free(bf->read_buffer);
    free(bf->write_buffer);
    free(bf);
    return result;
}
