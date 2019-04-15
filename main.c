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
#define CMP "comp.out"
#define CMP_PATH "./comp.out"
#define RESULT_OF_STU_TXT "res_of_running.txt"
#define NO_C_FILE "NO_C_FILE"
#define NO_C_FILE_GRADE "0"

#define COMPILE_ERROR "COMPILATION_ERROR"
#define COMPILE_ERROR_GRADE "20"

#define TIME_OUT "TIMEOUT"
#define TIME_OUT_GRADE "40"

#define BAD_OUT "BAD_OUTPUT"
#define BAD_OUT_GRADE "60"

#define SIM_OUT "SIMILAR_OUTPUT"
#define SIM_OUT_GRADE "80"

#define GOOD_OUT "GREAT_JOB"
#define GOOD_OUT_GRADE "100"

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
void cFile(char *path, char *nameOfFather);

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
void checkSemi(char *stu);

/**
 * write entry to the result file
 * @param name of student
 * @param grade of student
 * @param note of student
 */
void addEntry(char *name, char *grade, char *note);

void findOfOneChild(char *pathOfChild, DIR *dr, char *pathOfMain, char *_name);

void findInMainDir(char *pathOfMain);

//the csv of summeries the grades
int resFile = 0;
//the input of the program
int inputFileUser = 0;
//the right answers
int rightAnswers = 0;

char *rightRes;
char *inputFilePath;
int outRes = 0;//todo

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
    //open the config file
    int in;
    if ((in = open(argv[1], O_RDONLY)) == -1) {
        perror("cant open file\n");
        exit(-1);
    }
    char buffer[BUFFER_SIZE];
    char *path;
    char *input;
    char *right;
    //read the arguments
    read(in, buffer, BUFFER_SIZE);
    const char del[2] = {'\n', '\r'};
    path = strtok(buffer, del);
    input = strtok(NULL, del);
    right = strtok(NULL, del);
    close(in);

    inputFilePath = input;
    rightRes = right;

/*
    int inputFile;
    if ((inputFile = open(input, O_RDONLY)) == -1) {
        perror("ERROR inputFile\n");
        exit(-1);
    }
    inputFileUser = inputFile;
*/

    int results = open(RESULT, O_CREAT | O_RDWR | O_TRUNC, 0777);
    if (results < 0) {
        perror("ERROR IN RESULT\n");
        exit(-1);
    }
    resFile = results;


    int rightFile;
    if ((rightFile = open(right, O_RDONLY)) == -1) {
        //  close(inputFile);
        perror("ERROR right\n");
        exit(-1);
    }
    rightAnswers = rightFile;


    //find students in the main dir
    findInMainDir(path);

    //printf("end of pro of %d\n", getpid());
    //  close(inputFile);
    close(rightFile);
    close(resFile);
    return 0;
}


void findInMainDir(char *pathOfMain) {
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
            findOfOneChild(pathOfChild, newDir, newPathOfMain, de->d_name);
            if (pathOfChild[0] == 0) {//there isnt C file
                addEntry(de->d_name, NO_C_FILE_GRADE, NO_C_FILE);
                continue;
            } else {
                cFile(pathOfChild, de->d_name);
            }
        }
    }

}

void findOfOneChild(char *pathOfChild, DIR *dr, char *pathOfFather, char *studName) {
    if (dr == NULL) {
        return;
    }
    struct dirent *de;
    while ((de = readdir(dr)) != NULL) {//scan the stuFiles
        char newPathOfFather[SIZE_OF_LINE * 2];
        memset(newPathOfFather, 0, sizeof(newPathOfFather));
        strcat(newPathOfFather, pathOfFather);
        strcat(newPathOfFather, "/");
        strcat(newPathOfFather, de->d_name);
        if (de->d_type == 4) {
            if ((strcmp(de->d_name, ".") == 0) || (strcmp(de->d_name, "..") == 0)) {
                continue;
            }
            DIR *newDr = opendir(newPathOfFather);
            findOfOneChild(pathOfChild, newDr, newPathOfFather, studName);

        } else if (checkForC(de->d_name)) {
            strcpy(pathOfChild, newPathOfFather);
            return;
        }
    }//end of loop
}

void cFile(char *path, char *nameOfFather) {
    // printf("c file of:%s\n", path);//todo
    makeCompile(path);
    // printf("makeCompilereturn:%s\n", path);
    if (strcmp(path, COMPILE_ERROR) == 0) {//compilation error case
        addEntry(nameOfFather, COMPILE_ERROR_GRADE, COMPILE_ERROR);
        return;
    }
    runCFile(path);
    // printf("after run c :%s\n", path);
    if (strcmp(TIME_OUT, path) == 0) {//timeoutCase
        addEntry(nameOfFather, TIME_OUT_GRADE, TIME_OUT);
        return;
    }
    checkSemi(path);
    if (strcmp(GOOD_OUT, path) == 0) {
        addEntry(nameOfFather, GOOD_OUT_GRADE, GOOD_OUT);
        return;
    } else if (strcmp(BAD_OUT, path) == 0) {
        addEntry(nameOfFather, BAD_OUT_GRADE, BAD_OUT);
        return;
    } else if (strcmp(SIM_OUT, path) == 0) {
        addEntry(nameOfFather, SIM_OUT_GRADE, SIM_OUT);
        return;
    }

}

void checkSemi(char *stu) {

    pid_t pid;
    int status;

    //  open(RESULT_OF_STU_TXT,O_WRONLY|O_CREAT);
    pid = fork();
    if (pid < 0) {
        perror("ERROR in Check Semi\n");
        exit(-1);
    } else if (pid == 0) {//child
        execlp(CMP_PATH, CMP, RESULT_OF_STU_TXT, rightRes, NULL);
    } else {//dad
        if ((waitpid(pid, &status, 0)) < 0) {
            perror("error while calling wait()");
            exit(-1);
        }
        if (WIFEXITED(status)) {
            status = WEXITSTATUS(status);
        }
        memset(stu, 0, sizeof(stu));
        if (status == 1) {
            strcpy(stu, GOOD_OUT);
        } else if (status == 2) {
            strcpy(stu, BAD_OUT);
        } else if (status == 3) {
            strcpy(stu, SIM_OUT);
        } else {
            perror("Error in semi\n");
        }
    }

}

bool checkForC(char *name) {
    return (name[strlen(name) - 2] == '.' && ((name[strlen(name) - 1] == 'c') || (name[strlen(name) - 1] == 'C')));
}

void makeCompile(char *path) {
    int status, fail;
    pid_t pid;
    pid = fork();
    if (pid < 0) {
        perror("ERROR forking\n");
        exit(-1);
    } else if (pid == 0) {//child
        // printf("co of:%s\n", path);
        execlp("gcc", "gcc", path, "-o", STU_FILE_NAME, NULL);
    }  //dad
    if (wait(&status) < 0) {
        perror("CANT WAIT!\n");
        exit(-1);
    }
    if (WIFEXITED(status)) {
        fail = WEXITSTATUS(status);
        if (fail) {
            memset(path, 0, strlen(path));
            // printf("compile error\n");//todo
            strcpy(path, COMPILE_ERROR);
        }
    }
    kill(pid, SIGKILL);
}//end of MakeCompile


void runCFile(char *path) {
    //printf("start running\n");
    pid_t pid;
    int status;
    int fdin = open(inputFilePath, O_RDONLY);
    int fdout = open(RESULT_OF_STU_TXT, O_CREAT | O_RDWR | O_TRUNC, 0777);
    pid = fork();
    if (pid < 0) {
        perror("CANT FORK IN RUN\n");
        exit(-1);
    }
    if (pid == 0) {//child
        if (dup2(fdin, 0) < 0) {
            perror("DUP2in\n");
            exit(-1);
        }
        if (dup2(fdout, 1) < 0) {
            perror("DUP2out\n");
            exit(-1);
        }
        //execlp(STU_FILE_NAME_PATH, STU_FILE_NAME, NULL);
        execlp("./student.out", "student.out", NULL);
    }
    close(fdin);
    close(fdout);
    for (int i = 0; i < TIMEOUT; i++) {
        sleep(1);
        if ((waitpid(pid, &status, WNOHANG)) != 0) {//finish case
            //  kill(pid, SIGKILL);
            // printf("done without timeout\n");
            close(fdin);
            close(fdout);
            return;
        }
        // printf("loop %d\n", i);
    }
    memset(path, 0, sizeof(path));
    //printf("timeout\n");
    strcpy(path, "TIMEOUT");
    kill(pid, SIGSTOP);
    close(fdin);
    close(fdout);


}

int i = 1;

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

