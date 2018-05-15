#include <stdio.h>

#include "utils.h"

#define shmVarNMB 3

static char     *ptr_p;          // pointer in shm for pathname for ftok()
static int      *ptr_f;          // pointer in shm for fifo queue id client -> golibroda
static int      *ptr_pid;        // pointer in shm for pid of golibroda
static int      *ptr_chairs;     // pointer in shm for number of chairs
static int      *ptr_dream;      // pointer in shm for checking dream
static int      *ptr_queueLen;   // pointer in shm for lenght of queue
static int      *ptr_semID;      // pointer in shm for sem id
static int      *ptr_invitedID;   // pointer in shm for invited client ID
static int      *ptr_ifOnChair;  // pointer in shm with inf. that invited client is on chair or not

static int      *ptr_shmvar;

static int shmids[shmVarNMB];
static int shmidCounter = 0;
static int queueID;
static int semID;

static long sec;
static long msec;

static struct msgp buf;
static struct timespec time_info;
static struct sembuf sembuf_tab[1];


void time_point()
{
    int res;

    res = clock_gettime(CLOCK_MONOTONIC, &time_info );

    if(res == -1)
    {
        perror("time_point -> clock_gettime");
        exit(EXIT_FAILURE);
    }

    sec  = time_info.tv_sec/60;
    msec = time_info.tv_nsec/1000;
}

// set shared memory core
int set_shm(char **argv, int size, char keyChar)
{
    int shmid;
    key_t key;

    switch(keyChar)
    {
        case 'P':
            key = keyForPathname;
            break;
        default:
            key = ftok(argv[0], keyChar);
    }

    if(key == -1)
    {
        perror("set_shaGRN_pathname -> ftok");
        exit(EXIT_FAILURE);
    }


    shmid = shmget(key, size, S_IRWXU | IPC_CREAT);
    if(shmid == -1)
    {
        perror("set_shaGRN_pathname -> shmget");
        exit(EXIT_FAILURE);
    }

    return shmid;
}

// set shaGRN memory for pathname requiGRN in ftok()
int set_shm_for_pathname(char **argv)
{
    int shmid_p;
    char *tmp_ptr_p;

    shmid_p = set_shm(argv, 200, 'P'); // 'P' - pathname

    ptr_p   = shmat(shmid_p, NULL, 0);
    if(ptr_p == (char *)-1)
    {
        perror("shmat");
        exit(0);
    }

    tmp_ptr_p = ptr_p;
    strcpy(tmp_ptr_p, argv[0]);

    return shmid_p;
}

int set_shm_for_pid(char **argv)
{
    int shmid_pid;

    shmid_pid = set_shm(argv, sizeof(pid_t), PID_PROJ_ID);

    ptr_pid = shmat(shmid_pid, NULL, 0);
    if(ptr_pid == (pid_t *)-1)
    {
        perror("set_shm_for_fifo -> shmat");
        exit(EXIT_FAILURE);
    }

    *ptr_pid = getpid();

    return shmid_pid;
}

int set_shm_for_var_tab(char **argv)
{
    int shmid;

    shmid = set_shm(argv, SHMNMB * sizeof(int), SHMVAR_PROJ_ID);

    ptr_shmvar = shmat(shmid, NULL, 0);
    if(ptr_shmvar == (int *)-1)
    {
        perror("set_shm_for_var_tab -> shmat");
        exit(EXIT_FAILURE);
    }

    return shmid;
}

void initialize_shm_vars(char **argv)
{
    ptr_f          = &ptr_shmvar[0];
    ptr_chairs     = &ptr_shmvar[2];
    ptr_dream      = &ptr_shmvar[3];
    ptr_queueLen   = &ptr_shmvar[4];
    ptr_semID      = &ptr_shmvar[5];
    ptr_invitedID  = &ptr_shmvar[6];
    ptr_ifOnChair  = &ptr_shmvar[7];


    *ptr_f          = queueID;
    *ptr_chairs     = atoi(argv[1]);
    *ptr_dream      = 1;
    *ptr_queueLen   = 0;
    *ptr_semID      = semID;
    *ptr_invitedID  = 0;
    *ptr_ifOnChair  = 0;
}

// sets shaGRN memory for necessery variables
void set_shm_for_variables(char **argv)
{
    shmids[shmidCounter] = set_shm_for_pathname(argv);
    shmidCounter++;
    shmids[shmidCounter] = set_shm_for_pid(argv);
    shmidCounter++;
    shmids[shmidCounter] = set_shm_for_var_tab(argv);
    shmidCounter++;
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

    res = shmdt(ptr_p);
    if(res == -1)
    {
        perror("clean_shm -> shmdt(ptr_p)");
    }

    res = shmdt(ptr_shmvar);
    if(res == -1)
    {
        perror("clean_shm -> shmdt(ptr_shmvar)");
    }

    res = shmdt(ptr_pid);
    if(res == -1)
    {
        perror("clean_shm -> shmdt(ptr_pid)");
    }

    for (int i = 0; i < shmVarNMB; i++)
    {
        res = shmctl(shmids[i], IPC_RMID, NULL);
        if(res == -1)
        {
            perror("clean_shm -> shmctl");
        }
    }

}

void clean_workplace()
{
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
    key_t key = ftok(argv[0], SEM_PROJ_ID);
    if(key == -1)
    {
        perror("create_semaphore -> ftok");
        exit(EXIT_FAILURE);
    }

    semID = semget(key, SEM_NMB, S_IRUSR | S_IWUSR | IPC_CREAT);
    if(semID < 0)
    {
        perror("create_semaphore -> semget");
        exit(EXIT_FAILURE);
    }

    int res;
    union semun arg;

    arg.val = 0;
    res = semctl(semID, 0, SETVAL, arg);
    if(res == -1)
    {
        perror("create_semaphore -> semctl");
        exit(EXIT_FAILURE);
    }

    arg.val = 1;
    res = semctl(semID, 1, SETVAL, arg);
    if(res == -1)
    {
        perror("create_semaphore -> semctl");
        exit(EXIT_FAILURE);
    }

    arg.val = 1;
    res = semctl(semID, 2, SETVAL, arg);
    if(res == -1)
    {
        perror("create_semaphore -> semctl");
        exit(EXIT_FAILURE);
    }

    arg.val = 0;
    res = semctl(semID, 3, SETVAL, arg);
    if(res == -1)
    {
        perror("create_semaphore -> semctl");
        exit(EXIT_FAILURE);
    }

}

void take_semaphore(int sem)
{
    int res;

    sembuf_tab[0].sem_op = -1;
    sembuf_tab[0].sem_num = sem;
    sembuf_tab[0].sem_flg = 0;

    res = semop(*ptr_semID, sembuf_tab, 1);
    if(res == -1)
    {
        perror("take_semaphore -> semop");
        exit(EXIT_FAILURE);
    }
}

void give_semaphore(int sem)
{
    int res;

    sembuf_tab[0].sem_op = 1;
    sembuf_tab[0].sem_num = sem;
    sembuf_tab[0].sem_flg = 0;

    res = semop(*ptr_semID, sembuf_tab, 1);
    if(res == -1)
    {
        perror("give_semaphore -> semop");
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

void invite_client(int who)
{

    take_from_queue();
    int clientID = buf.pid;

    time_point();
    printf(RED "Zaproszenie klienta: " RES BLU "%d" RES GRN " [%ld:%ld]\n" RES, clientID, sec, msec);
    *ptr_invitedID = clientID;

    // wait until client is in chair
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
                invite_client(FROM_QUEUE);
                cut();
                break;
        }

    }

    return 0;
}
