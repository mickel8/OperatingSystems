#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>


// max number of arguments that can be given to task in file
static int argNmb = 7; // one for the name of instruction and one for NULL

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

void interpret_tasks(char *fileName){

    char *line;
    char **args;
    int exitStat;
    int execRes;
    char **envs = get_path_var_value();

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
            if(execv(args[0], args) == -1)
                execvp(args[0], args);
            exit(EXIT_FAILURE);
        }
        else{
            if(wait(&exitStat) == -1){
                perror("interpret_tasks()");
                return;
            }
        }

        if(exitStat == 0)
            line = read_line(&file);
        else{
            printf("interpret_tasks() - error during executing: %s\n", line);
            return;
        }
    }

    close_file(file);
    if(errno) return;
    
}

int main(int argc, char **argv){

    if(argc != 2){
        printf("Bad number of arguments\n");
        return 0;
    }

    char *fileName = argv[1];

    interpret_tasks(fileName);

    return 0;
}