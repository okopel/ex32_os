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
#include <dirent.h>
#include <stdbool.h>
#include <memory.h>
#include <sys/wait.h>
#include <sys/stat.h>


#define SIZE_OF_LINE 151
#define BUFFER_SIZE 460
#define TIMEOUT 5
#define RESULT "results.csv"
#define STU_FILE_NAME "student.out"
#define STU_FILE_NAME_PATH "./student.out"
#define STD_OUTPUT "output_of_student"
#define CMP "comp.out"
#define CMP_PATH "./comp.out"

void findInDir(DIR *dr, char *father, char *right);

void makeCompile(char *path);

void cFile(char *path, char *right, char *name);

void runCFile(char *path);

bool checkForC(char *name);

void checkSemi(char *stu, char *right);

void addEntry(char *name, char *grade, char *note);

int resFile = 0;

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
        perror("cant open file\n");
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

    int inputFile;
    if ((inputFile = open(input, O_RDONLY)) == -1) {
        close(in);
        perror("ERROR inputFile\n");
        exit(-1);
    }
    int rightFile;
    if ((rightFile = open(right, O_RDONLY)) == -1) {
        close(in);
        close(inputFile);
        perror("ERROR right\n");
        exit(-1);

    }
    getcwd(path, sizeof(path));
    DIR *dr = opendir(path);
    if (dr == NULL) {
        perror("NULL DIR\n");
        exit(-1);
    }

    int results = open(RESULT, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (results < 0) {
        perror("ERROR IN RESULT\n");
        exit(-1);
    }
    resFile = results;
    findInDir(dr, NULL, right);


    closedir(dr);
    close(inputFile);
    close(rightFile);
    close(in);
    return 0;
}


void findInDir(DIR *dr, char *father, char *right) {
    if (dr == NULL) {
        return;
    }
    struct dirent *de;
    while ((de = readdir(dr)) != NULL) {
        if (de->d_type == 4) {
            if ((father != NULL && strcmp(de->d_name, father) == 0) || (strcmp(de->d_name, ".") == 0) ||
                (strcmp(de->d_name, "..") == 0)) {
                continue;
            }
            DIR *newDir = opendir(de->d_name);
            findInDir(newDir, de->d_name, right);
            closedir(newDir);
            continue;
        }
        if (checkForC(de->d_name)) {
            char buf[SIZE_OF_LINE];
            realpath(father, buf);
            strcat(buf, "/");
            strcat(buf, de->d_name);
            cFile(buf, right, father);
        }
    }
}

void cFile(char *path, char *right, char *name) {
    makeCompile(path);
    if (strcmp(path, "COMPILATION_ERROR") == 0) {//compilation error case
        addEntry(name, "20", "COMPILATION_ERROR");
        return;
    }
    runCFile(path);
    if (strcmp("TIMEOUT", path)) {//timeoutCase
        addEntry(name, "40", "TIMEOUT");
        return;
    }
    checkSemi(path, right);
    if (strcmp("GREAT_JOB", path)) {
        addEntry(name, "100", "GREAT_JOB");
        return;
    } else if (strcmp("BAD_OUTPUT", path)) {
        addEntry(name, "60", "BAD_OUTPUT");
        return;
    } else if (strcmp("SIMILAR_OUTPUT", path)) {
        addEntry(name, "80", "SIMILAR_OUTPUT");
        return;
    }

}

void checkSemi(char *stu, char *right) {
    pid_t pid;
    int status;
    pid = fork();
    if (pid < 0) {
        perror("ERROR in Check Semi\n");
        exit(-1);
    } else if (pid == 0) {//child
        execlp(CMP_PATH, CMP, STD_OUTPUT, right, NULL);
    } else {//dad
        if ((waitpid(pid, &status, 0)) < 0) {
            perror("error while calling wait()");
            exit(-1);
        }
        if (WIFEXITED(status)) {
            status = WEXITSTATUS(status);
        }
        if (status == 1) {
            strcpy(stu, "GREAT_JOB");
        } else if (status == 2) {
            strcpy(stu, "BAD_OUTPUT");
        } else if (status == 3) {
            strcpy(stu, "SIMILAR_OUTPUT");
        }
    }

}

bool checkForC(char *name) {
    return (name[strlen(name) - 2] == '.' && name[strlen(name) - 1] == 'c');
}

void makeCompile(char *path) {
    int status, fail;
    pid_t pid = fork();
    if (pid < 0) {
        perror("ERROR forking\n");
        exit(-1);
    } else if (pid == 0) {//child
        execlp("gcc", "gcc", path, "-o", STU_FILE_NAME, NULL);
    } else {//dad
        if ((wait(&status) < 0)) {
            perror("CANT WAIT!\n");
            exit(-1);
        }
        if (WIFEXITED(status)) {
            fail = WEXITSTATUS(status);
            if (fail) {
                memset(path, 0, strlen(path));
                strcpy(path, "COMPILATION_ERROR");
            }
        }

    }//end of dad

}//end of MakeCompile
void runCFile(char *path) {
    pid_t pid;
    int status;
    pid = fork();
    if (pid < 0) {
        perror("CANT FORK IN RUN\n");
        exit(-1);
    } else if (pid == 0) {//child
        execlp(STU_FILE_NAME_PATH, STU_FILE_NAME, NULL);

    } else {//dad
        sleep(TIMEOUT);
        if ((waitpid(pid, &status, WNOHANG)) != 0) {//finish case
            return;
        } else {
            strcpy(path, "TIMEOUT");
            kill(pid, SIGSTOP);
        }
    }
}

void addEntry(char *name, char *grade, char *note) {
    char entry[BUFFER_SIZE];
    strcat(entry, name);
    strcat(entry, ",");
    strcat(entry, grade);
    strcat(entry, ",");
    strcat(entry, note);
    strcat(entry, "\n");
    if (write(resFile, entry, sizeof(entry)) < 0) {
        perror("CANT SAVE\n");
        exit(-1);
    }
}

