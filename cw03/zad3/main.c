#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <errno.h>
#define COLOR  "\x1B[36m"
#define RESET "\x1B[0m"


// max number of arguments that can be given to task in file
static int argNmb = 7; // one for the name of instruction and one for NULL
static struct rusage usage;
FILE * open_file(char *fileName){

    FILE *file = fopen(fileName, "r");
    if(!file){
        perror("open_file()");
        return NULL;
    }

    return file;

}

void close_file(FILE *file){
    int res = fclose(file);
    if(res != 0){
        perror("close_file()");
        return;
    }
}

char **get_path_var_value(){
    
    char **envs = (char**)calloc(1, sizeof(char *));
    envs[0] = getenv("PATH");
    
    return envs;
}

char *read_line(FILE **file){
    
    char *lineptr = NULL;
    long int n = 0;
    
    int readBytes = getline(&lineptr, &n, *file);
    if(readBytes == -1){
        if(errno) perror("read_line()");
        return NULL;
    }
    
    return lineptr;
}

void clean_nl_char(char *line){
    char *nPtr = strrchr(line, '\n');
    if(nPtr != NULL) *nPtr = '\0';
}

char **split_args(char *line){
    
    char **args = (char **)calloc(argNmb, sizeof(char *));
    args[0] = strtok(line, " ");
    for(int i = 1; i < argNmb; i++){
        args[i] = strtok(NULL, " ");
    }
    args[argNmb - 1] = NULL;

    return args;
} 

void print_usage(){
    printf(COLOR "* ZASOBY *\n" RESET "USER TIME\n-tv_sec: %ld\n-tv_usec: %ld\nSYSTEM TIME\n-tv_sec: %ld\n-tv_usec: %ld\n", 
    usage.ru_utime.tv_sec,
    usage.ru_utime.tv_usec,
    usage.ru_stime.tv_sec,
    usage.ru_stime.tv_sec);
}

void interpret_tasks(char *fileName, int cpuLim, int memLim){

    char *line;
    char **args;
    int exitStat;
    int execRes;
    char **envs = get_path_var_value();

    // setting values of limits to fileds in structs
    struct rlimit newCPU;
    struct rlimit newMEM;
    newCPU.rlim_max = cpuLim;
    newCPU.rlim_cur = cpuLim/2;
    newMEM.rlim_max = memLim;
    newMEM.rlim_cur = memLim/2;

    FILE *file = open_file(fileName);
    if(errno) return;
    
    line = read_line(&file);
    while(line != NULL){
       
        clean_nl_char(line); // remove \n from the end of line
        args = split_args(line);

        pid_t childPid = fork();
        if(childPid < 0){
            perror("interpret_tasks()");
            return;
        }
        if(childPid == 0){

            if(setrlimit(RLIMIT_AS, &newMEM) == -1){
                perror("error during setting MEM limit");
                exit(2);
            }

            if(setrlimit(RLIMIT_CPU, &newCPU) == -1){
                perror("error during setting CPU limit");
                exit(2);
            }


            if(execv(args[0], args) == -1){
                if(execvp(args[0], args) == -1)
                    perror("exec");
            }
            exit(EXIT_FAILURE);
        }
        else if(childPid > 0){
            if(wait3(&exitStat, 0, &usage) == -1){
                perror("interpret_tasks()");
                return;
            }
            else if(exitStat != 0 ){
                //printf("%d\n", exitStat);
                if(errno){
                    perror("child process exited");
                }
                if(exitStat != 512)
                    printf("interpret_tasks() - error during executing: %s\n", line);
                return;
            }
            else{
                print_usage();
                line = read_line(&file);
            }
        }
    }

    close_file(file);
    if(errno) return;
    
}

int main(int argc, char **argv){

    if(argc != 4){
        printf("Bad number of arguments\n");
        return 0;
    }

    char *fileName = argv[1];
    int cpuLim = atoi(argv[2]);
    int memLim = atoi(argv[3]) * 1000000; // given in MB, needed in bytes
    
    interpret_tasks(fileName, cpuLim, memLim);

    return 0;
}