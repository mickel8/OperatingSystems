#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

// max number of tasks in one line in file
#define tasksNmb 5

// max number of arguments that can be given to task in file
static int argNmb = 7; // one for the name of instruction and one for NULL
static int pipes[5][2];
static int pids[tasksNmb];

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
    long unsigned int n = 0;
    
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

char **split_tasks(char *line){
    char **tasks = (char **)calloc(tasksNmb, sizeof(char *));
    tasks[0] = strtok(line, "|");
    for(int i = 1; i < tasksNmb; i++){
        tasks[i] = strtok(NULL, "|");
    }
    
    
    return tasks;
}

void interpret_tasks(char *fileName){

    char *line;
    char **args;
    char **tasks;
    int exitStat;
  
    FILE *file = open_file(fileName);
    if(errno) return;
    
    line = read_line(&file);
    while(line != NULL){
       
        clean_nl_char(line); // remove \n from the end of line
        tasks = split_tasks(line);

        // first process without changing stdin
        if(pipe(pipes[0]) == -1){
            perror("pipe()");
            exit(EXIT_FAILURE);
        }

        args = split_args(tasks[0]);
        
        pid_t childPid = fork();
        if(childPid < 0){
            perror("fork()");
            exit(EXIT_FAILURE);
        }
        else if(childPid == 0){
            
            // *STDOUT to PIPE
            if(dup2(pipes[0][1], STDOUT_FILENO) == -1){
                perror("dup2()");
                exit(EXIT_FAILURE);
            }
            if(close(pipes[0][0]) == -1){
                perror("close()");
                exit(EXIT_FAILURE);
            }
            if(close(pipes[0][1]) == -1){
                perror("close()");
                exit(EXIT_FAILURE);
            }
            if(execv(args[0], args) == -1)
                execvp(args[0], args);
            perror("exec1()");
            exit(-1);
        }

        int i = 1;
        for(i = 1; i < tasksNmb; i++){

            if(tasks[i] == NULL) break;
            
            if(pipe(pipes[i]) == -1){
                perror("pipe()");
                exit(EXIT_FAILURE);
            }

            args = split_args(tasks[i]);
            
            pid_t childPid = fork();
            
            if(childPid < 0){
                perror("interpret_tasks()");
                exit(EXIT_FAILURE);
            }
            else if(childPid == 0){

                if((i < tasksNmb - 1  && tasks[i+1] == NULL) || i == tasksNmb - 1){
                    
                    if(dup2(pipes[i-1][0], STDIN_FILENO) == -1){
                        perror("dup2()");
                        exit(EXIT_FAILURE);
                    }
                }
                else{
                    if(dup2(pipes[i-1][0], STDIN_FILENO) == -1){
                        perror("dup2()");
                        exit(EXIT_FAILURE);
                    }
                    if(dup2(pipes[i][1], STDOUT_FILENO) == -1){
                        perror("dup2()");
                        exit(EXIT_FAILURE);
                    }
                }

                if(close(pipes[i-1][0]) == -1){
                    perror("close()");
                    exit(EXIT_FAILURE);
                }
                if(close(pipes[i-1][1]) == -1){
                    perror("close()");
                    exit(EXIT_FAILURE);
                }
                if(close(pipes[i][0]) == -1){
                    perror("close()");
                    exit(EXIT_FAILURE);
                }
                if(close(pipes[i][1]) == -1){
                    perror("close()");
                    exit(EXIT_FAILURE);
                }

                if(execv(args[0], args) == -1)
                    execvp(args[0], args);
                perror("exec()2");
                exit(-1);
            }
           
            // parent

            if(close(pipes[i-1][0]) == -1){
                perror("close()");
                exit(EXIT_FAILURE);
            }
            if(close(pipes[i-1][1]) == -1){
                perror("close()");
                exit(EXIT_FAILURE);
            }

        }

        int j = 0;
        while(j < tasksNmb ){
            if(waitpid(pids[j], &exitStat, 0) == -1){
                if(errno == ECHILD) errno = 0;
                else{
                    perror("waitpid()");
                    exit(EXIT_FAILURE);
                }
            }

            if(WIFEXITED(exitStat)){         
                if(WEXITSTATUS(exitStat) == -1){   
                    printf("interpret_tasks() - error during executing: %s\n", tasks[j]);
                    exit(EXIT_FAILURE);
                }
            }
            else{ // *
                printf("interpret_tasks() - error during executing: %s\n", tasks[j]);
                exit(EXIT_FAILURE);
            }

            j++;
        }
        
        if(close(pipes[i-1][0]) == -1){
            perror("close()");
            exit(EXIT_FAILURE);
        }
        if(close(pipes[i-1][1]) == -1){
            perror("close()");
            exit(EXIT_FAILURE);
        }

        line = read_line(&file);

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