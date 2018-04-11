#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

static pid_t childPid;
static union sigval value;
static int existFlag;

void send_signal(int sigNo){
    
    int killStatus;

    printf("Proces macierzysty - otrzymałem sygnał SIGTSTP \n");
    
    if(existFlag == 1){
        killStatus = kill(childPid, SIGTSTP);
        if(killStatus != 0){
            perror("kill() - ");
            exit(EXIT_FAILURE);
        }
        else 
            existFlag = 0;
    }
    else if(existFlag == 0){
         
        childPid = fork();
        if(childPid == 0){
            int execRes = execlp("sh", "sh", "myDate.sh", NULL);
            if(execRes == -1){
                perror("exec() - ");
                exit(EXIT_FAILURE);
            }
        }
        else if(childPid > 0)
            existFlag = 1;
        else if(childPid < 0){
            perror("fork() - \n");
            exit(EXIT_FAILURE);
        }
    }

    
}   

void int_action(int sigNo){
    
    printf("Proces macierzysty - otrzymałem sygnał SIGINT \n");
    int killStatus;
    killStatus = kill(childPid, SIGINT);
    if(killStatus != 0){
        perror("kill() - ");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv){
   
    int exitStat;

    childPid = fork();
    
    if(childPid == 0){
        execlp("sh", "sh", "myDate.sh", NULL);
        // below only if exec fails
        perror("exec() - ");
        exit(EXIT_FAILURE);
        
    }
    else if(childPid > 0){

        existFlag = 1;   // means that process exists 

        struct sigaction act;
        act.sa_handler = send_signal;
        sigemptyset(&act.sa_mask);
        act.sa_flags = SA_NODEFER;
        
        struct sigaction intAct;
        intAct.sa_handler = int_action;
        sigemptyset(&intAct.sa_mask);
        intAct.sa_flags = 0;

        sigaction(SIGTSTP, &act, NULL);
        sigaction(SIGINT, &intAct, NULL);

        while(1) {
        }


    }
    else if(childPid < 0){
        perror("fork() - \n");
        exit(EXIT_FAILURE);
    }


    return 0;
}