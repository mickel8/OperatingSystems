#include <stdio.h>

#include "utils.h"

static char     *ptr_p;          // pointer in shm for pathname for ftok()
static int      *ptr_f;          // pointer in shm for fifo queue id client -> golibroda
static int      *ptr_chairs;     // pointer in shm for number of chairs
static int      *ptr_dream;      // pointer in shm for checking dream
static int      *ptr_queueLen;   // pointer in shm for lenght of queue
static int      *ptr_semID;      // pointer in shm for sem id
static int      *ptr_invitedID;   // pointer in shm for invited client ID
static int      *ptr_ifOnChair;   // pointer in shm with inf. that invited client is on chair or not

static int      *ptr_shmvar;


static long sec;
static long msec;

static struct msgp buf;
static struct timespec time_info;
static struct sembuf sembuf_tab[1];

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

    sec  = time_info.tv_sec/60;
    msec = time_info.tv_nsec/1000;
}

void get_pathname()
{
    int shmid;

    shmid = shmget(keyForPathname, 200, S_IRUSR | S_IWUSR);
    if(shmid == -1)
    {
        perror("c: get_pathname -> shmget");
        exit(EXIT_FAILURE);
    }

    ptr_p = shmat(shmid, NULL, 0);
    if(ptr_p == (char *) -1)
    {
        perror("c: get_pathname -> shmat");
        exit(EXIT_FAILURE);
    }

}

void get_shm_var_tab()
{

    int shmid;
    key_t key;

    key = ftok(ptr_p, SHMVAR_PROJ_ID);

    shmid = shmget(key, SHMNMB * sizeof(int), S_IRUSR | S_IWUSR);
    if(shmid == -1)
    {
        perror("get_shm_var_tab -> shmget");
        exit(EXIT_FAILURE);
    }

    ptr_shmvar = shmat(shmid, NULL, 0);
    if(ptr_shmvar == (int *) -1)
    {
        perror("get_shm_var_tab -> shmat");
        exit(EXIT_FAILURE);
    }

}

void clean_workplace()
{
    int res;

    res = shmdt(ptr_p);
    if(res == -1)
    {
        perror("clean_shm -> shmdt(ptr_p)");
    }

}

void initialize_ptrs_for_shm_vars()
{
    ptr_f          = &ptr_shmvar[0];
    ptr_chairs     = &ptr_shmvar[1];
    ptr_dream      = &ptr_shmvar[2];
    ptr_queueLen   = &ptr_shmvar[3];
    ptr_semID      = &ptr_shmvar[4];
    ptr_invitedID  = &ptr_shmvar[5];
    ptr_ifOnChair  = &ptr_shmvar[6];
}

void get_shm_variables()
{
    get_pathname();
    get_shm_var_tab();
    initialize_ptrs_for_shm_vars();
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
    exit(EXIT_SUCCESS);
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
    give_semaphore(SEM_Q);
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
    int nmbOfHaircut = atoi(argv[1]);

    get_shm_variables();


    res = atexit(clean_workplace);
    if(res != 0)
    {
        perror("main -> atexit");
        exit(EXIT_FAILURE);
    }


    for(int i = 0; i < nmbOfHaircut; i++)
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
                        break;
                }
                break;
            case 1:
                wake();
                break;
        }
        sit_in_chair();
        wait_for_the_end();
        go_out_end_cutting();
    }

    exit(EXIT_SUCCESS);
}
