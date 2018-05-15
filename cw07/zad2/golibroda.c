#include <stdio.h>

#include "utils.h"

static int      *ptr_f;          // pointer in shm for fifo queue id client -> golibroda
static pid_t    *ptr_pid;        // pointer in shm for pid of golibroda
static int      *ptr_chairs;     // pointer in shm for number of chairs
static int      *ptr_dream;      // pointer in shm for checking dream
static int      *ptr_queueLen;   // pointer in shm for lenght of queue
static int      *ptr_invitedID;  // pointer in shm for invited client ID
static int      *ptr_ifOnChair;  // pointer in shm with inf. that invited client is on chair or not
static int      *ptr_shmvar;

static int queueID;

static long sec;
static long msec;

static struct msgp buf;
static struct timespec time_info;

static sem_t *sem_q;
static sem_t *sem_fas;
static sem_t *sem_c;

static int pid_fd;
static int vartab_fd;


void time_point()
{
    int res;

    res = clock_gettime(CLOCK_MONOTONIC, &time_info );

    if(res == -1)
    {
        perror("time_point -> clock_gettime");
        exit(EXIT_FAILURE);
    }

    sec  = time_info.tv_sec;
    msec = time_info.tv_nsec/1000;
}

void set_shm_for_pid()
{
    int res;

    res = shm_open(PID_SHM, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR);
    if(res == -1)
    {
        perror("set_shm_for_pid -> shm_open");
        exit(EXIT_FAILURE);
    }

    pid_fd = res;

    res = ftruncate(pid_fd, sizeof(pid_t));
    if(res == -1)
    {
        perror("set_shm_for_pid -> ftruncate");
        exit(EXIT_FAILURE);
    }


    ptr_pid = mmap(NULL, sizeof(pid_t), PROT_READ | PROT_WRITE, MAP_SHARED, pid_fd, 0);
    if(ptr_pid == (pid_t *) -1)
    {
        perror("set_shm_for_pid -> mmap");
        exit(EXIT_FAILURE);
    }


    *ptr_pid = getpid();

}

void set_shm_for_var_tab()
{
    int res;

    res = shm_open(VARTAB_SHM, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR);
    if(res == -1)
    {
        perror("set_shm_for_var_tab -> shm_open");
        exit(EXIT_FAILURE);
    }

    vartab_fd = res;

    res = ftruncate(vartab_fd, SHMNMB * sizeof(int));
    if(res == -1)
    {
        perror("set_shm_for_var_tab -> ftruncate");
        exit(EXIT_FAILURE);
    }


    ptr_shmvar = mmap(NULL, SHMNMB * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, vartab_fd, 0);
    if(ptr_shmvar == (int *) -1)
    {
        perror("set_shm_for_var_tab -> mmap");
        exit(EXIT_FAILURE);
    }

}

void initialize_shm_vars(char **argv)
{
    ptr_f          = &ptr_shmvar[0];
    ptr_chairs     = &ptr_shmvar[1];
    ptr_dream      = &ptr_shmvar[2];
    ptr_queueLen   = &ptr_shmvar[3];
    ptr_invitedID  = &ptr_shmvar[4];
    ptr_ifOnChair  = &ptr_shmvar[5];


    *ptr_f          = queueID;
    *ptr_chairs     = atoi(argv[1]);
    *ptr_dream      = 1;
    *ptr_queueLen   = 0;
    *ptr_invitedID  = 0;
    *ptr_ifOnChair  = 0;
}

// sets shaGRN memory for necessery variables
void set_shm_for_variables(char **argv)
{
    set_shm_for_pid();
    set_shm_for_var_tab();
    initialize_shm_vars(argv);
}

// handler for SIGTERM signal
void SIGTERM_handler()
{
    printf("Otrzymałem sygnał SIGTERM, sprzatam zakład i kończę pracę na dziś\n");
    exit(EXIT_SUCCESS);
}

// sets handler for SIGTERM signal
void handle_SIGTERM_signal()
{
    int res;

    struct sigaction act;
    act.sa_handler = SIGTERM_handler;
    act.sa_flags = 0;

    res = sigaction(SIGTERM, &act, NULL);
    if(res == -1)
    {
        perror("handle_SIGTERM_signal -> sigaction");
        exit(EXIT_FAILURE);
    }

    res = sigaction(SIGINT, &act, NULL);
    if(res == -1)
    {
        perror("handle_SIGTERM_signal -> sigaction");
        exit(EXIT_FAILURE);
    }

}

// blocks all signal except SIGTERM signal
void block_signals()
{
    int res;
    sigset_t set;

    res = sigfillset(&set);
    if(res == -1)
    {
        perror("block_signals -> sigfillset");
        exit(EXIT_FAILURE);
    }

    res = sigdelset(&set, SIGTERM);
    if(res == -1)
    {
        perror("block_signals -> sigdelset");
        exit(EXIT_FAILURE);
    }

    res = sigprocmask(SIG_SETMASK, &set, NULL);
    if(res == -1)
    {
        perror("block_signals -> sigprockmask");
        exit(EXIT_FAILURE);
    }

}

// creates queue for clinets
void create_queue(char **argv)
{
    int res;
    key_t key;

    key = ftok(argv[0], QUEUE_PROJ_ID);
    if(key == -1)
    {
        perror("creat_queue -> ftok");
        exit(EXIT_FAILURE);
    }

    res = msgget(key, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if(res == -1)
    {
        perror("creat_queue -> msgget");
        exit(EXIT_FAILURE);
    }
    else
    {
        queueID = res;
    }

}

// deletes clients queue
void delete_queue()
{
    int res;

    res = msgctl(queueID, IPC_RMID, NULL);
    if(res == -1)
    {
        perror("delete_queue -> msgctl");
        exit(EXIT_FAILURE);
    }

}

void clean_shm()
{
    int res;

    res = munmap(ptr_shmvar, SHMNMB * sizeof(int));
    if(res == -1) perror("clean_shm -> munmap");

    res = shm_unlink(VARTAB_SHM);
    if(res == -1) perror("clean_shm -> shm_unlink");
}

void close_sems()
{
    int res;

    const char *nq = "sem_q";
    const char *nfas = "sem_fas";
    const char *nc = "sem_c";


    res = sem_close(sem_q);
    if(res == -1) perror("close_sems -> sem_close(sem_q)");

    res = sem_close(sem_fas);
    if(res == -1) perror("close_sems -> sem_close(sem_fas)");

    res = sem_close(sem_c);
    if(res == -1) perror("close_sems -> sem_close(sem_c)");

    res = sem_unlink(nq);
    if(res == -1) perror("close_sems -> sem_unlink(sem_q)");

    res = sem_unlink(nfas);
    if(res == -1) perror("close_sems -> sem_unlink(sem_fas)");

    res = sem_unlink(nc);
    if(res == -1) perror("close_sems -> sem_unlink(sem_c)");

}

void clean_workplace()
{
    close_sems();
    delete_queue();
    clean_shm();
}

void take_from_queue()
{
    int res;

    res = msgrcv(*ptr_f, &buf, sizeof(buf) - sizeof(long), 0, IPC_NOWAIT);
    if(res == -1)
    {
        perror("take_from_queue -> msgrcv");
        exit(EXIT_FAILURE);
    }
}

void create_semaphore(char **argv)
{
    const char *nq = "sem_q";
    const char *nfas = "sem_fas";
    const char *nc = "sem_c";

    sem_q   = sem_open(nq, O_CREAT, S_IRUSR | S_IWUSR, 1);
    if(sem_q == SEM_FAILED)
    {
        perror("create_semaphore -> nq");
        exit(EXIT_FAILURE);
    }

    sem_fas = sem_open(nfas, O_CREAT, S_IRUSR | S_IWUSR, 0);
    if(sem_fas == SEM_FAILED)
    {
        perror("create_semaphore -> nfas");
        exit(EXIT_FAILURE);
    }

    sem_c   = sem_open(nc, O_CREAT, S_IRUSR | S_IWUSR, 0);
    if(sem_c == SEM_FAILED)
    {
        perror("create_semaphore -> nc");
        exit(EXIT_FAILURE);
    }

    // int valp;
    // sem_getvalue(sem_q, &valp);
    // printf("%d\n", valp);
    //
    // sem_getvalue(sem_fas, &valp);
    // printf("%d\n", valp);
    //
    // sem_getvalue(sem_c, &valp);
    // printf("%d\n", valp);


}

void take_semaphore(int sem)
{
    int res;

    switch(sem)
    {
        case SEM_Q:
            res = sem_wait(sem_q);
            break;
        case SEM_FAS:
            res = sem_wait(sem_fas);
            break;
        case SEM_C:
            res = sem_wait(sem_c);
            break;
    }

    if(res == -1)
    {
        perror("take_semaphore -> sem_wait");
        exit(EXIT_FAILURE);
    }

}

void give_semaphore(int sem)
{
    int res;

    switch(sem)
    {
        case SEM_Q:
            res = sem_post(sem_q);
            break;
        case SEM_FAS:
            res = sem_post(sem_fas);
            break;
        case SEM_C:
            res = sem_post(sem_c);
            break;
    }

    if(res == -1)
    {
        perror("give_semaphore -> sem_post");
        exit(EXIT_FAILURE);
    }
}

int check_queue()
{

    take_semaphore(SEM_Q);

    int res;
    struct msqid_ds buf;
    msgqnum_t msg_qnum;

    res = msgctl(queueID, IPC_STAT, &buf);

    if(res == -1)
    {
        perror("check_queue -> msgctl");
        exit(EXIT_FAILURE);
    }

    msg_qnum = buf.msg_qnum;

    if(msg_qnum > 0) return 0;
    else return -1;


}

void invite_client()
{

    take_from_queue();
    int clientID = buf.pid;

    time_point();
    printf(RED "Zaproszenie klienta: " RES BLU "%d" RES GRN " [%ld:%ld]\n" RES, clientID, sec, msec);
    *ptr_invitedID = clientID;

    // wait until client is not in chair
    while(*ptr_ifOnChair != clientID)
    {

    }

    *ptr_queueLen = *ptr_queueLen - 1;

    give_semaphore(SEM_Q);

}

void cut()
{
    int clientID = buf.pid;

    time_point();
    printf("Rozpoczęcie strzyżenia klienta: %d" GRN " [%ld:%ld]\n" RES, clientID, sec, msec);
    time_point();
    printf("Zakończenie strzyżenia klienta: %d" GRN " [%ld:%ld]\n" RES, clientID, sec, msec);

    *ptr_invitedID = 0;
    give_semaphore(SEM_C);

    while(*ptr_ifOnChair != 0)
    {

    }
}

void fall_asleep()
{
    time_point();
    printf("Zaśnięcie Golibrody" GRN " [%ld:%ld]\n" RES, sec, msec);
    *ptr_dream = 1;

    give_semaphore(SEM_Q);
    take_semaphore(SEM_FAS);

    time_point();
    printf("Obudzenie Golibrody" GRN " [%ld:%ld]\n" RES, sec, msec);

    while(*ptr_ifOnChair == 0)
    {
        //
    }

    time_point();
    printf("Rozpoczęcie strzyżenia klienta: " BLU "%d" RES GRN " [%ld:%ld]\n" RES, *ptr_invitedID, sec, msec);
    time_point();
    printf("Zakończenie strzyżenia klienta: " BLU "%d" RES GRN " [%ld:%ld]\n" RES, *ptr_invitedID, sec, msec);

    give_semaphore(SEM_C);

}


int main(int argc, char **argv)
{

    if(argc != 2)
    {
        printf("Bad number of arguments\n");
        exit(EXIT_FAILURE);
    }

    printf("My pid is: %d\n", getpid());

    int res;

    create_queue(argv);

    res = atexit(clean_workplace);
    if(res != 0)
    {
        perror("main -> atexit");
        exit(EXIT_FAILURE);
    }


    //block_signals();
    handle_SIGTERM_signal();
    create_semaphore(argv);
    set_shm_for_variables(argv);

    while(1)
    {

        res = check_queue();
        switch(res)
        {
            case -1:
                fall_asleep();
                break;
            case 0:
                invite_client();
                cut();
                break;
        }

    }

    return 0;
}
