#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>

#define MEMBERS 4096
#define DATE_NMBR 40

static pid_t pid;

char *prepare_pid_info(){
    char tmp[5];
    char *pidStr = calloc(10, sizeof(char));
    if(sprintf(tmp, "%d", pid) < 0){
        perror("sprintf()");
        exit(EXIT_FAILURE);
    }

    strcpy(pidStr, "Pid: ");
    strcat(pidStr, tmp);
    strcat(pidStr, ", ");

    return pidStr;
}

int main(int argc, char **argv){

    char content[MEMBERS];
    char *pidStr;
    char *dateptr; 
    FILE *date;

    if(argc != 3){
        printf("Bad number of arguments");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    char *pipe = argv[1];
    int N = atoi(argv[2]);

    if(access(pipe, W_OK) == -1){
        printf("There is no pipe of given name\n");
        exit(EXIT_FAILURE);
    }
    
    struct stat stats;
    if(lstat(pipe, &stats) == -1){
        perror("lstat()");
        exit(EXIT_FAILURE);
    }

    if(!S_ISFIFO(stats.st_mode)){
        printf("File of given name is not a named pipe\n");
        exit(EXIT_FAILURE);
    } 

    FILE *file = fopen(pipe, "w");
    if(file == NULL){
        perror("fopen()");
        exit(EXIT_FAILURE);
    }

    pid = getpid();
    printf("Pid of slave program: %d\n", pid);

    pidStr = prepare_pid_info();

    dateptr = calloc(DATE_NMBR, sizeof(char));

    for(int i = 0; i < N; i++){
        
        date = popen("date", "r");
        if(date == NULL || date == -1){
            perror("popen()");
            exit(EXIT_FAILURE);
        }

        if(fgets(dateptr, DATE_NMBR, date) == NULL && errno){
            perror("fgets()");
            exit(EXIT_FAILURE);
        }
        
        strcpy(content, pidStr);
        strcat(content, dateptr);

        if(fputs(content, file) == EOF){
            perror("fputs()");
            exit(EXIT_FAILURE);
        }

        sleep((rand()%4) + 2);
    }
    if(fclose(file) == EOF){
        perror("fclose()");
        exit(EXIT_FAILURE);
    }
    

    return 0;
}