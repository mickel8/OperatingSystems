#define _POSIX_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#define clear() printf("\033[H\033[J") // clear bash screen
void pause_program(int);

static time_t timeUpToNow;
static struct tm *timeInfo;
static int sigstpFlag = 0;
struct sigaction stopAct;

void print_time(){

    while(1){
        time(&timeUpToNow); // obsluga bledow?
        timeInfo = localtime(&timeUpToNow);
        printf("Time: %d:%d:%d\n", timeInfo -> tm_hour, timeInfo -> tm_min, timeInfo -> tm_sec);
        sleep(1);
        clear();
    }
}

void do_sigint(int sigNo){
    printf("Odebrano sygnał SIGINT\n");
    raise(SIGKILL);
}

void pause_program(int sigNo){

    if(sigstpFlag == 0){
        sigstpFlag = 1;
        printf("\nOczekuję na CTRL+Z - kontynuacja\nalbo\nCTRL+C - zakończenie programu\n");
        pause();
    }
    else if(sigstpFlag == 1){
        sigstpFlag = 0;
        raise(SIGCONT);
    }
    
}

int main(int argc, char **argv){    
    
    stopAct.sa_handler = pause_program;
    sigemptyset(&stopAct.sa_mask);
    stopAct.sa_flags = SA_NODEFER;

    signal(SIGINT, do_sigint);
    sigaction(SIGTSTP, &stopAct, NULL);

    print_time();

    return 0;
}