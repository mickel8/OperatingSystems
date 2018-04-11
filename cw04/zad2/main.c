#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

void send_rtsig(int);
void recive_usrSig(int, siginfo_t *, void *);

static sigset_t usrSig;
struct sigaction parentAct;
static struct sigaction act;
static int n, k;
static int *processQueue;
static int *children;
static int *rtSignals;
static int recivedRequests = 0;
static int reachedRequests = 0;
static int recivedRTSignals = 0;
static int sentPermits = 0;
static int sleepTime = 0;

// PRINTING INFORMATION FUNCTIONS //

void print_child_pid(int child){
    printf("Child pid: %d\n", child);
}
void print_recived_requests(){
    printf("Number of recived signals: %d\n", recivedRequests);
}
void print_sent_permits(){
    printf("Number of sent permits: %d\n", sentPermits);
}
void print_recived_rt_signals(){
    printf("Number of recived real time signals: %d\n", recivedRTSignals);
    for(int i = 0; i < n, rtSignals[i] != 0; i++){
        printf("%d. %d\n", i, rtSignals[i]);
    }
}
void print_child_exit_status(int child, int status){
    printf("Status of child: %d is: %d\n", child, status);
}

// MAIN FUNCTIONS //

void create_sigaction_parentAct(){
    sigemptyset(&parentAct.sa_mask);
    parentAct.sa_flags = SA_SIGINFO;
    parentAct.sa_sigaction = recive_usrSig;
}
void create_sigaction_act(){

    act.sa_handler = send_rtsig;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
}
void create_sigset_usrSig(){

    if(sigfillset(&usrSig) != 0){
        perror("sigfillset() - ");
        exit(EXIT_FAILURE);
    }
    if(sigdelset(&usrSig, SIGUSR1) != 0){
        perror("sigdelset() - ");
        exit(EXIT_FAILURE);
    }
}

int check_if_child(int child){
    
    for(int i = 0; i < n; i++){
        if(children[i] == child) return 1;
    }
    return 0;
}

void send_rtsig(int sigNo){
    
    int x = rand() % 33;
    if(kill(getppid(), SIGRTMIN + x) != 0){
        perror("send_rtsig() -> kill() - ");
        printf("PPID: %d SIGNAL: %d\n", getppid(), SIGRTMIN + x);
        exit(EXIT_FAILURE);
    }
    exit(sleepTime);
}
void recive_kill_signal(int sigNo){
    printf("Otrzymałem sygnał 2 from: %d\n", getpid());
    for(int i = 0; i < n; i++){
        if(children[i] != 0){
            if(kill(children[i], 0) != -1 && errno != ESRCH){
                printf("Zamykam proces: %d\n", children[i]);
                if(kill(children[i], SIGKILL) == -1){
                    perror("recive_kill_signal() -> kill2() - ");
                    exit(EXIT_FAILURE);
                }
            } 
        }
    }
    exit(EXIT_SUCCESS);
}
void recive_rtsig(int sigNo, siginfo_t *sigInfo, void *contex){

    if(check_if_child(sigInfo -> si_pid) == 0) return;
    rtSignals[recivedRTSignals] = sigNo;
    recivedRTSignals ++;
}
void recive_usrSig(int sigNo, siginfo_t *sigInfo, void *contex){


    if(check_if_child(sigInfo -> si_pid) == 0) return;
    
    processQueue[recivedRequests] = sigInfo -> si_pid; // add process to queue with waiting processes
    recivedRequests ++; // increment number of recived signals
    int status;

    if(reachedRequests == 1){
        if(kill(sigInfo -> si_pid, SIGUSR1) != 0){
            perror("recive_usrSig() -> kill1() - ");
            exit(EXIT_FAILURE);
        }
        else{
            sentPermits ++;
        }

        while(waitpid(sigInfo -> si_pid, &status, 0) == -1){
            if(errno == EINTR) continue;
            else{
                perror("recive_usrSig() -> waitpid1() - ");
                exit(EXIT_FAILURE);
            }
        }
        if(WIFEXITED(status))
            printf("IE: ");
            print_child_exit_status(sigInfo -> si_pid, WEXITSTATUS(status));
        
    }
    else if(recivedRequests >= k){
        reachedRequests = 1;
        for(int i = 0; i < recivedRequests; i++){
            
            if(kill(processQueue[i], SIGUSR1) != 0){
                perror("recive_usrSig() -> kill2() - ");
                exit(EXIT_FAILURE);
            }
            else{
                sentPermits ++;
            }
                
            while(waitpid(processQueue[i], &status, 0) == -1){
                if(errno == EINTR) continue;
                else{
                    perror("recive_usrSig() -> waitpid2() - ");
                    exit(EXIT_FAILURE);
                }
            }
            if(WIFEXITED(status))
                print_child_exit_status(sigInfo -> si_pid, WEXITSTATUS(status));
        }
    }
    
}

void create_processes(){

    children = (int *)calloc(n, sizeof(int));
    pid_t child;

    for(int i = 0; i < n; i++){
        child = fork();
        if(child > 0){
            children[i] = child;
            print_child_pid(child);
            pause();
        }
        else if(child == 0){
            
            signal(SIGINT, SIG_DFL);
            srand(time(NULL)^(getpid() << 16));

            sleepTime = rand()%11;
            sleep(sleepTime);
            
            
            // only for usage of SA_SIGINFO in sigaction
            union sigval value;

            if(sigqueue(getppid(), SIGUSR1, value) != 0){
                perror("kill() - ");
                exit(EXIT_FAILURE);
            }

            create_sigaction_act();
            if(sigaction(SIGUSR1, &act, NULL) == -1){
                perror("sigaction() - ");
                exit(EXIT_FAILURE);
            }

            if(sigsuspend(&usrSig) == -1){
                perror("sigsuspend() - ");
                exit(EXIT_FAILURE);
            }
        }
        else if(child < 0){
            perror("fork() - ");
            exit(EXIT_FAILURE);
        }
    }

    print_recived_rt_signals();
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv){


    printf("%d\n", getpid());

    if(argc != 3){
        printf("Bad number of arguments\n");
        exit(EXIT_FAILURE);
    }
    
    n = atoi(argv[1]);
    k = atoi(argv[2]);
    processQueue = calloc(k, sizeof(int));
    rtSignals = calloc(n, sizeof(int));

    create_sigaction_parentAct();
    sigaction(SIGUSR1, &parentAct, NULL);

    for(int i = SIGRTMIN; i <= SIGRTMAX; i++) {
        parentAct.sa_sigaction = recive_rtsig;
        if(sigaction(i, &parentAct, NULL) == -1){
            perror("sigaction() - ");
            exit(EXIT_FAILURE);
        }
    }

    struct sigaction intAct;
    intAct.sa_flags =0;
    sigemptyset(&intAct.sa_mask);
    intAct.sa_handler = recive_kill_signal;
    
    if(sigaction(SIGINT, &intAct, NULL) == -1){
        perror("sigaction() - ");
        exit(EXIT_FAILURE);
    }
    
    create_sigset_usrSig();
    create_processes();

    return 0;
}