#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib_tar.h"

/**
 * You are free to use this file to write tests for your implementation
 */

void debug_dump(const uint8_t *bytes, size_t len) {
    for (int i = 0; i < len;) {
        printf("%04x:  ", (int) i);

        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", bytes[i + j]);
        }
        printf("\t");
        for (int j = 0; j < 16 && i < len; j++, i++) {
            printf("%c ", bytes[i]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s tar_file\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1] , O_RDONLY);
    if (fd == -1) {
        perror("open(tar_file)");
        return -1;
    }
   // modificar para que haga lo de las carpetas
   
    int ret_check = check_archive(fd);
    printf("check_archive returned %d\n", ret_check);

    int ret_exists = exists(fd,"lib_tar.h");
    printf("exists returned %d\n", ret_exists);

    int ret_dir = is_dir(fd,"lib_tar.c");
    printf("check_archive returned %d\n", ret_dir);

    int ret_file = is_file(fd,"lib_tar.c");
    printf("is_file returned %d\n", ret_file);


    return 0;
}