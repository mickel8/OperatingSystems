#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

static int L;
static int Type;
static int recived_L = 0;
static int recived_child_L = 0;
static int sent_L = 0;
static pid_t child;

void print_recived_L(){
    printf("Recived USR1 signals by child: %d\n", recived_L);
}
void print_recived_child_L(){
    printf("Recived USR1 signals by parent: %d\n", recived_child_L);
}
void print_sent_L(){
    printf("Sent USR1 signals by parent to child: %d\n", sent_L);
}


void recive_int_signal(int sigNo){
    //printf("Otrzymałem sygnał 2\n");
    if(kill(child, SIGUSR2) == -1){
        perror("create_process_type_1() -> kill()2");
        exit(EXIT_FAILURE);
    }
    int status;
    if(waitpid(child, &status, 0) == -1){
        perror("recive_int_signal() -> waitpid()");
        exit(EXIT_FAILURE);
    }

    if(Type == 2){
        print_sent_L();
        print_recived_child_L();
    }
    exit(EXIT_SUCCESS);
}

void recive_sigusr_1(int sigNo){

    recived_L++;
    if(kill(getppid(), SIGUSR1) == -1){
        perror("create_process_type_1() -> kill1()");
        exit(EXIT_FAILURE);
    }

}

void recive_sigusr_2(int sigNo){
    //printf("Otrzymałem SIGUSR2\n");
    print_recived_L();
    exit(EXIT_SUCCESS);
}

void recive_child_sigusr_1(int sigNo){
    recived_child_L++;
}

struct sigaction create_sigaction_act(){
    struct sigaction act;
    act.sa_handler = recive_sigusr_1;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    return act;
}

void create_process_type_1(){
    
    child = fork();

    if(child == 0){

        struct sigaction act;
        act.sa_handler = recive_sigusr_1;
        sigset_t newMask;
        sigfillset(&newMask);
        if(sigdelset(&act.sa_mask, SIGINT) == -1){
            perror("create_process_type_1() -> sigdelset1()");
            exit(EXIT_FAILURE);
        }
        if(sigdelset(&act.sa_mask, SIGUSR1) == -1){
            perror("create_process_type_1() -> sigdelset2()");
            exit(EXIT_FAILURE);
        }
        if(sigdelset(&act.sa_mask, SIGUSR2) == -1){
            perror("create_process_type_1() -> sigdelset()");
            exit(EXIT_FAILURE);
        }
        act.sa_mask = newMask;
        act.sa_flags = 0;


        if(sigaction(SIGUSR1, &act, NULL) == -1){
            perror("create_process_type_1() -> sigaction1()");
            exit(EXIT_FAILURE);
        }

        act.sa_handler = recive_sigusr_2;
        if(sigaction(SIGUSR2, &act, NULL) == -1){
            perror("create_process_type_1() -> sigaction2()");
            exit(EXIT_FAILURE);
        }

        while(1){

        }

    }
    else if (child > 0){
        

        struct sigaction act = create_sigaction_act();
        act.sa_handler = recive_child_sigusr_1;
        if(sigaction(SIGUSR1, &act, NULL) == -1){
            perror("create_process_type_1() -> sigaction3()");
            exit(EXIT_FAILURE);
        }

        act.sa_handler = recive_int_signal;
        if(sigaction(SIGINT, &act, NULL) == -1){
            perror("create_process_type_1() -> sigaction4()");
            exit(EXIT_FAILURE);
        }


        sleep(1);

        // L USR1 signals
        for(int i = 0; i < L; i++){
            if(kill(child, SIGUSR1) == -1){
                perror("create_process_type_1() -> kill1()");
                exit(EXIT_FAILURE);
            }
            else{
                sent_L++;
            }
        }

        // USR2 signal
        if(kill(child, SIGUSR2) == -1){
            perror("create_process_type_1() -> kill()2");
            exit(EXIT_FAILURE);
        }

        int status;
        while(waitpid(child, &status, 0) == -1){
            if(errno == EINTR) continue;
            else{
                perror("recive_usrSig() -> waitpid1() - ");
                exit(EXIT_FAILURE);
            }
        }

        print_sent_L();
        print_recived_child_L();
        exit(EXIT_SUCCESS);

    }
    else if (child < 0){
        perror("create_process() -> fork()");
        exit(EXIT_FAILURE);
    }
}

void create_process_type_2(){
    child = fork();

    if(child == 0){

        struct sigaction act;
        act.sa_handler = recive_sigusr_1;
        sigset_t newMask;
        sigfillset(&newMask);
        if(sigdelset(&act.sa_mask, SIGINT) == -1){
            perror("create_process_type_1() -> sigdelset1()");
            exit(EXIT_FAILURE);
        }
        if(sigdelset(&act.sa_mask, SIGUSR1) == -1){
            perror("create_process_type_1() -> sigdelset2()");
            exit(EXIT_FAILURE);
        }
        if(sigdelset(&act.sa_mask, SIGUSR2) == -1){
            perror("create_process_type_1() -> sigdelset()");
            exit(EXIT_FAILURE);
        }
        act.sa_mask = newMask;
        act.sa_flags = 0;


        if(sigaction(SIGUSR1, &act, NULL) == -1){
            perror("create_process_type_1() -> sigaction1()");
            exit(EXIT_FAILURE);
        }

        act.sa_handler = recive_sigusr_2;
        if(sigaction(SIGUSR2, &act, NULL) == -1){
            perror("create_process_type_1() -> sigaction2()");
            exit(EXIT_FAILURE);
        }

        while(1){

        }

    }
    else if (child > 0){
        

        struct sigaction act = create_sigaction_act();
        act.sa_handler = recive_child_sigusr_1;
        if(sigaction(SIGUSR1, &act, NULL) == -1){
            perror("create_process_type_1() -> sigaction3()");
            exit(EXIT_FAILURE);
        }

        act.sa_handler = recive_int_signal;
        if(sigaction(SIGINT, &act, NULL) == -1){
            perror("create_process_type_1() -> sigaction4()");
            exit(EXIT_FAILURE);
        }


        sleep(1);

        // L USR1 signals
        
        sigset_t set;
        if(sigfillset(&set) == -1){
            perror("create_process_type_2() -> sigfillset()");
            exit(EXIT_FAILURE);
        }
        if(sigdelset(&set, SIGUSR1) == -1){
            perror("create_process_type_2() -> sigdelset()");
        }

        for(int i = 0; i < L; i++){
            if(kill(child, SIGUSR1) == -1){
                perror("create_process_type_1() -> kill1()");
                exit(EXIT_FAILURE);
            }
            else{
                sent_L++;
                sigsuspend(&set);
            }
        }

        // USR2 signal
        if(kill(child, SIGUSR2) == -1){
            perror("create_process_type_1() -> kill()2");
            exit(EXIT_FAILURE);
        }

        int status;
        while(waitpid(child, &status, 0) == -1){
            if(errno == EINTR) continue;
            else{
                perror("recive_usrSig() -> waitpid1() - ");
                exit(EXIT_FAILURE);
            }
        }

        print_sent_L();
        print_recived_child_L();
        exit(EXIT_SUCCESS);

    }
    else if (child < 0){
        perror("create_process() -> fork()");
        exit(EXIT_FAILURE);
    }
}

void create_process_type_3(){

}

int main(int argc, char **argv){


    if(argc != 3){
        printf("Bad number of arguments\n)");
        exit(EXIT_FAILURE);
    }

    L = atoi(argv[1]);
    Type = atoi(argv[2]);
    
    if(Type != 1 && Type != 2 && Type != 3){
        printf("Bad type number\n");
        exit(EXIT_FAILURE);
    }

    if(L <= 0){ 
        printf("Bad value of first argument\n");
        exit(EXIT_FAILURE);
    }

    if(Type == 1) create_process_type_1();
    else if(Type == 2) create_process_type_2();
    else if(Type == 3) create_process_type_3();

    return 0;
}