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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#define SIZE_OF_LINE 151
#define BUFFER_SIZE 10

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
    char buffer[BUFFER_SIZE];
    char path[SIZE_OF_LINE];
    char input[SIZE_OF_LINE];
    char right[SIZE_OF_LINE];

    int in;
    if ((in = open(argv[2], O_RDONLY)) == -1) {
        printf("ERROR\n");
        exit(-1);
    }
    size_t reader = 0;
    int index = 0;
    int subIndex = 0;
    int status = 1;
    char c;
    while (1) {
        if ((index % BUFFER_SIZE) == 0) {
            reader = read(in, buffer, sizeof(buffer));
            if (reader < 0) {
                break;
            }
        }
        c = buffer[index & BUFFER_SIZE];
        if (c == '\n') {
            status++;
            index++;
            subIndex = 0;
            continue;
        }
        if (status == 1) {
            path[subIndex++] = c;
        } else if (status == 2) {
            input[subIndex++] = c;
        } else if (status == 3) {
            right[subIndex++] = c;
        }

    }//end of while


    return 0;
}

#pragma clang diagnostic pop