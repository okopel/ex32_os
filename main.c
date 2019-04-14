/**
 * Ori Kopel
 * 205533151
 * Ex3_2
 * April 19
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#define SIZE_OF_LINE 151
#define BUFFER_SIZE 460

/**
 *
 * @param argc 3
 * @param argv [1]=name, [2]=path
 * @return
 */
int main(int argc, char **argv) {
    if (argc != 3) {
        perror("PARAMS ERROR\n");
    }
    int in;
    if ((in = open(argv[2], O_RDONLY)) == -1) {
        printf("ERROR\n");
        exit(-1);
    }
    char buffer[BUFFER_SIZE];
    char *path;
    char *input;
    char *right;
    read(in, buffer, 480);
    const char del[2] = {'\n', '\r'};
    path = strtok(buffer, del);
    input = strtok(NULL, del);
    right = strtok(NULL, del);

    int pathFile;
    if ((pathFile = open(path, O_RDONLY)) == -1) {
        close(in);
        printf("ERROR\n");
        exit(-1);
    }
    int inputFile;
    if ((inputFile = open(input, O_RDONLY)) == -1) {
        close(in);
        close(pathFile);
        printf("ERROR\n");
        exit(-1);
    }
    int rightFile;
    if ((rightFile = open(right, O_RDONLY)) == -1) {
        close(in);
        close(pathFile);
        close(inputFile);
        printf("ERROR\n");
        exit(-1);
    }


    close(pathFile);
    close(inputFile);
    close(rightFile);
    close(in);
    return 0;
}

#pragma clang diagnostic pop