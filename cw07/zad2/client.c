#include <stdio.h>

#include "utils.h"

static int      *ptr_f;           // pointer in shm for fifo queue id client -> golibroda
static int      *ptr_chairs;      // pointer in shm for number of chairs
static int      *ptr_dream;       // pointer in shm for checking dream
static int      *ptr_queueLen;    // pointer in shm for lenght of queue
static int      *ptr_invitedID;   // pointer in shm for invited client ID
static int      *ptr_ifOnChair;   // pointer in shm with inf. that invited client is on chair or not
static int      *ptr_shmvar;

static sem_t *sem_q;
static sem_t *sem_fas;
static sem_t *sem_c;

static int vartab_fd;

static long sec;
static long msec;

static struct msgp buf;
static struct timespec time_info;

void sit_in_chair(void);



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

void get_shm_var_tab()
{
    int res;

    res = shm_open(VARTAB_SHM, O_RDWR, S_IWUSR | S_IRUSR);
    if(res == -1)
    {
        perror("get_shm_var_tab -> shm_open");
        exit(EXIT_FAILURE);
    }

    vartab_fd = res;

    ptr_shmvar = mmap(NULL, SHMNMB * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, vartab_fd, 0);
    if(ptr_shmvar== (int *)-1)
    {
        perror("get_shm_var_tab -> mmap");
        exit(EXIT_FAILURE);
    }

}

void clean_workplace()
{
    int res;

    res = munmap(ptr_shmvar, SHMNMB * sizeof(int));
    if(res == -1)
    {
        perror("clean_workplace -> munmap");
        exit(EXIT_FAILURE);
    }

}

void initialize_ptrs_for_shm_vars()
{
    ptr_f          = &ptr_shmvar[0];
    ptr_chairs     = &ptr_shmvar[1];
    ptr_dream      = &ptr_shmvar[2];
    ptr_queueLen   = &ptr_shmvar[3];
    ptr_invitedID  = &ptr_shmvar[4];
    ptr_ifOnChair  = &ptr_shmvar[5];
}

void get_shm_variables()
{
    get_shm_var_tab();
    initialize_ptrs_for_shm_vars();
}

void get_sems()
{
    const char *nq = "sem_q";
    const char *nfas = "sem_fas";
    const char *nc = "sem_c";

    sem_q   = sem_open(nq, O_RDWR);
    if(sem_q == SEM_FAILED)
    {
        perror("get_sems -> nq");
        exit(EXIT_FAILURE);
    }

    sem_fas = sem_open(nfas, O_RDWR);
    if(sem_fas == SEM_FAILED)
    {
        perror("get_sems -> nfas");
        exit(EXIT_FAILURE);
    }

    sem_c   = sem_open(nc, O_RDWR);
    if(sem_c == SEM_FAILED)
    {
        perror("get_sems -> nc");
        exit(EXIT_FAILURE);
    }
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


int check_golibrodas_state()
{
    take_semaphore(SEM_Q);
    return *ptr_dream;
}

int seek_free_sit()
{
    if((*ptr_chairs - (*ptr_queueLen)) <= 0) return 0;
    else return 1;
}

void go_out_no_chairs()
{
    time_point();
    printf("Opuszczenie zakładku z powodu braku wolnych miejsc w poczekalni: %d [%ld:%ld]\n", getpid(), sec, msec);
    give_semaphore(SEM_Q);
}

void go_out_end_cutting()
{
    time_point();
    printf("Opuszczenie zakładu po zakończeniu strzyżenia: %d [%ld:%ld]\n", getpid(), sec, msec);
    *ptr_ifOnChair = 0;
}

void wait_in_queue()
{

    time_point();
    printf("Zajęcie miejsca w poczekalni: %d [%ld:%ld]\n", getpid(), sec, msec);

    int res;

    buf.mtype = 1;
    buf.pid = getpid();

    res = msgsnd(*ptr_f, &buf, sizeof(buf) - sizeof(long), IPC_NOWAIT);
    if(res == -1)
    {
        perror("wait_in_queue -> msgsnd");
        exit(EXIT_FAILURE);
    }

    *ptr_queueLen = *ptr_queueLen + 1;


    give_semaphore(SEM_Q);

    while(1)
    {
        if(*ptr_invitedID == getpid() && *ptr_ifOnChair == 0) break;
    }

}

void wake()
{

    time_point();
    printf("Obudzenie Golibrody: %d [%ld:%ld]\n", getpid(), sec, msec);
    *ptr_invitedID = getpid();
    *ptr_dream = 0;

    give_semaphore(SEM_FAS);
}

void sit_in_chair()
{
    time_point();
    printf(RED "Zajęcie miejsca na krześle do strzyżenia: " RES BLU "%d" RES GRN "[%ld:%ld]" RES "\n" , getpid(), sec, msec);
    *ptr_ifOnChair = getpid();
}

void wait_for_the_end()
{
    take_semaphore(SEM_C);
}


int main(int argc, char **argv)
{
    int res;
    int res2;
    int visits = 0;
    int nmbOfHaircut = atoi(argv[1]);

    get_shm_variables();
    get_sems();


    res = atexit(clean_workplace);
    if(res != 0)
    {
        perror("main -> atexit");
        exit(EXIT_FAILURE);
    }

    while(1)
    {

        res = check_golibrodas_state();
        switch(res)
        {
            case 0:
                res2 = seek_free_sit();
                switch(res2)
                {
                    case 0:
                        go_out_no_chairs();
                        break;
                    case 1:
                        wait_in_queue();
                        sit_in_chair();
                        wait_for_the_end();
                        visits++;
                        go_out_end_cutting();
                        break;
                }
                break;
            case 1:
                wake();
                sit_in_chair();
                give_semaphore(SEM_Q);
                wait_for_the_end();
                visits++;
                go_out_end_cutting();
                break;
        }

        if(visits >= nmbOfHaircut) break;
    }

    exit(EXIT_SUCCESS);
}
