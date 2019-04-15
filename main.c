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
#define STD_OUTPUT "./output_of_student"
#define CMP "comp.out"
#define CMP_PATH "./comp.out"

/**
 * Recursiv func to find in dir
 * @param dr source dir
 * @param pathOfFather father of this dir(=name)
 * @param right the eight answers
 */
void findInDir(DIR *dr, char *pathOfFather, char *nameOfFather, char *right);

/**
 * compile c file
 * @param path the path of the c file
 */
void makeCompile(char *path);

/**
 * work on c file
 * @param path to the file
 * @param right the right answers
 * @param nameOfFather of the file
 */
void cFile(char *path, char *right, char *nameOfFather);

/**
 * run the c file
 * @param path path
 */
void runCFile(char *path);
/**
 * check if file is c file
 * @param name name of file
 * @return true if c file
 */
bool checkForC(char *name);

/**
 * run the comp.out program
 * @param stu the student file
 * @param right the right answers
 */
void checkSemi(char *stu, char *right);

/**
 * write entry to the result file
 * @param name of student
 * @param grade of student
 * @param note of student
 */
void addEntry(char *name, char *grade, char *note);

void findOfOneChild(char *pathOfChild, DIR *dr, char *pathOfMain, char *_name, char *rightAnswers);

void findInMainDir(char *pathOfMain, char *rightAnswers);

int resFile = 0;

/**
 *
 * @param argc 2
 * @param argv [1]=path
 * @return
 */
int main(int argc, char **argv) {
    if (argc < 2) {
        perror("PARAMS ERROR\n");
    }
    int in;
    if ((in = open(argv[1], O_RDONLY)) == -1) {
        perror("cant open file\n");
        exit(-1);
    }
    char buffer[BUFFER_SIZE];
    char *path;
    char *input;
    char *right;
    read(in, buffer, BUFFER_SIZE);
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
    int results = open(RESULT, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (results < 0) {
        perror("ERROR IN RESULT\n");
        exit(-1);
    }
    resFile = results;

    int rightFile;
    if ((rightFile = open(right, O_RDONLY)) == -1) {
        close(in);
        close(inputFile);
        perror("ERROR right\n");
        exit(-1);
    }

    findInMainDir(path, right);
    printf("end of pro of %d\n", getpid());
    //  closedir(dr);
    close(inputFile);
    close(rightFile);
    close(in);
    return 0;
}

void findInMainDir(char *pathOfMain, char *rightAnswers) {
    //   getcwd(pathOfMain, sizeof(pathOfMain));
    DIR *dr = opendir(pathOfMain);
    if (dr == NULL) {
        perror("CANT OPEN PATH\n");
        exit(-1);
    }
    struct dirent *de;
    while ((de = readdir(dr)) != NULL) {//move over students
        if (de->d_type == 4) {//folderCase
            if ((strcmp(de->d_name, pathOfMain) == 0) || (strcmp(de->d_name, ".") == 0) ||
                (strcmp(de->d_name, "..") == 0)) {
                continue;
            }
            char pathOfChild[2 * SIZE_OF_LINE];
            memset(pathOfChild, 0, sizeof(pathOfChild));
            char newPathOfMain[2 * SIZE_OF_LINE];
            memset(newPathOfMain, 0, sizeof(newPathOfMain));
            strcat(newPathOfMain, pathOfMain);
            strcat(newPathOfMain, "/");
            strcat(newPathOfMain, de->d_name);
            DIR *newDir = opendir(newPathOfMain);
            findOfOneChild(pathOfChild, newDir, newPathOfMain, de->d_name, rightAnswers);
            if (pathOfChild[0] == 0) {//there isnt C file
                addEntry(de->d_name, "0", "NO_C_FILE");
                continue;
            } else {
                cFile(pathOfChild, rightAnswers, de->d_name);

            }
        }
    }

}

void findInDir(DIR *dr, char *pathOfFather, char *nameOfFather, char *right) {
    if (dr == NULL) {
        return;
    }
    bool flag = false;
    struct dirent *de;
    while ((de = readdir(dr)) != NULL) {
        //  printf("look at: %s\n", de->d_name);//todo
        if (de->d_type == 4) {
            if ((pathOfFather != NULL && strcmp(de->d_name, pathOfFather) == 0) || (strcmp(de->d_name, ".") == 0) ||
                (strcmp(de->d_name, "..") == 0)) {
                continue;
            }
            char newDirPath[SIZE_OF_LINE * 2];
            memset(newDirPath, 0, strlen(newDirPath));
            strcat(newDirPath, pathOfFather);
            strcat(newDirPath, "/");
            strcat(newDirPath, de->d_name);
            //realpath(pathOfFather, newDirPath);
            // strcat(newDirPath, "/");
            //strcat(newDirPath, de->d_name);
            //  printf("new Dir:%s\n", newDirPath);//todo
            DIR *newDir = opendir(newDirPath);
            if (newDir == NULL) {
                perror("cant enter to dir \n");
            }
            //printf("enter to dir %s\n", newDirPath);
            findInDir(newDir, newDirPath, de->d_name, right);
            closedir(newDir);
            continue;
        }//end of type 4=dir
        if (checkForC(de->d_name)) {
            flag = true;
            char buf[SIZE_OF_LINE];
            memset(buf, 0, strlen(buf));
            strcat(buf, pathOfFather);
            strcat(buf, "/");
            strcat(buf, de->d_name);
            //  printf("we found C file named: %s\t\t, our pathOfFather is: %s\n", buf, pathOfFather);
            cFile(buf, right, nameOfFather);
        }
    }//end of while
    if ((!flag) && (pathOfFather != NULL)) {
        addEntry(nameOfFather, "0", "NO_C_FILE");
    }
}//end of findDir

void findOfOneChild(char *pathOfChild, DIR *dr, char *pathOfFather, char *studName, char *rightAnswers) {
    if (dr == NULL) {
        pathOfChild = NULL;
        return;
    }
    struct dirent *de;
    while ((de = readdir(dr)) != NULL) {//scan the stuFiles
        char newPathOfFather[SIZE_OF_LINE * 2];
        memset(newPathOfFather, 0, sizeof(newPathOfFather));
        strcat(newPathOfFather, pathOfFather);
        strcat(newPathOfFather, "/");
        strcat(newPathOfFather, de->d_name);
        // printf("this file:%s\n", newPathOfFather);
        if (de->d_type == 4) {
            if ((strcmp(de->d_name, ".") == 0) || (strcmp(de->d_name, "..") == 0)) {
                continue;
            }
            DIR *newDr = opendir(newPathOfFather);
            findOfOneChild(pathOfChild, newDr, newPathOfFather, studName, rightAnswers);

        } else if (checkForC(de->d_name)) {
            strcpy(pathOfChild, newPathOfFather);
            return;
        }
    }//end of loop
}

void cFile(char *path, char *right, char *nameOfFather) {
    makeCompile(path);
    if (strcmp(path, "COMPILATION_ERROR") == 0) {//compilation error case
        addEntry(nameOfFather, "20", "COMPILATION_ERROR");
        return;
    }
    runCFile(path);
    if (strcmp("TIMEOUT", path) == 0) {//timeoutCase
        addEntry(nameOfFather, "40", "TIMEOUT");
        return;
    }
    checkSemi(path, right);
    if (strcmp("GREAT_JOB", path) == 0) {
        addEntry(nameOfFather, "100", "GREAT_JOB");
        return;
    } else if (strcmp("BAD_OUTPUT", path) == 0) {
        addEntry(nameOfFather, "60", "BAD_OUTPUT");
        return;
    } else if (strcmp("SIMILAR_OUTPUT", path) == 0) {
        addEntry(nameOfFather, "80", "SIMILAR_OUTPUT");
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
        kill(pid, SIGKILL);
        printf("%s SATUS!!\n", status);
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
    return (name[strlen(name) - 2] == '.' && ((name[strlen(name) - 1] == 'c') || (name[strlen(name) - 1] == 'C')));
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
                //    printf("compile error\n");//todo
                strcpy(path, "COMPILATION_ERROR");
            }
        }
        kill(pid, SIGKILL);
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
        for (int i = 0; i < TIMEOUT; i++) {
            sleep(1);
            if ((waitpid(pid, &status, WNOHANG)) != 0) {//finish case
                return;
            }
        }
        memset(path, 0, sizeof(path));
        strcpy(path, "TIMEOUT");
        kill(pid, SIGKILL);
    }
}

int i = 0;

void addEntry(char *name, char *grade, char *note) {
    char entry[BUFFER_SIZE];
    memset(entry, 0, strlen(entry));
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
    printf("%d)we saved  the entey: %s", i++, entry);
}

