#include <stdio.h>

#include "utils.h"

static char     *ptr_p;    // pointer in shm for pathname for ftok()
static int      *ptr_f;    // pointer in shm for fifo queue id
static pid_t    *ptr_pid;  // pointer in shm for pid of golibroda

static int queueID;

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
        perror("set_shared_pathname -> ftok");
        exit(EXIT_FAILURE);
    }


    shmid = shmget(key, size, S_IRWXU | IPC_CREAT);
    if(shmid == -1)
    {
        perror("set_shared_pathname -> shmget");
        exit(EXIT_FAILURE);
    }

    return shmid;
}

// set shared memory for pathname required in ftok()
void set_shm_for_pathname(char **argv)
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
}

// set shared memory for fifo queue id
void set_shm_for_queueID(char **argv)
{
    int shmid_f;

    shmid_f = set_shm(argv, sizeof(int), QUEUE_PROJ_ID);

    ptr_f = shmat(shmid_f, NULL, 0);
    if(ptr_f == (int *)-1)
    {
        perror("set_shm_for_fifo -> shmat");
        exit(EXIT_FAILURE);
    }

    *ptr_f = queueID;

}

void set_shm_for_pid(char **argv)
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

}

// sets shared memory for necessery variables
void set_shm_for_variables(char **argv)
{
    set_shm_for_pathname(argv);
    set_shm_for_queueID(argv);
    set_shm_for_pid(argv);
}

// handler for SIGTERM signal
void SIGTERM_handler()
{
    printf("Otrzymałem sygnał SIGTERM, kończę pracę na dziś\n");
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
    else
    {
        printf("Sprzątam zakład - usuwam kolejkę\n");
    }
}




int main(int argc, char **argv)
{

    int res;

    printf("My pid is: %d\n", getpid());

    create_queue(argv);

    res = atexit(delete_queue);
    if(res != 0)
    {
        perror("main -> atexit");
        exit(EXIT_FAILURE);
    }



    //block_signals();
    handle_SIGTERM_signal();
    set_shm_for_variables(argv);


    while(1)
    {

    }

    return 0;
}
