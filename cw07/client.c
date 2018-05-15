#include <stdio.h>

#include "utils.h"

static char     *ptr_p;          // pointer in shm for pathname for ftok()
static int      *ptr_f;          // pointer in shm for fifo queue id client -> golibroda
static int      *ptr_f_client;   // pointer in shm for fifo queue id golibroda -> client
static int      *ptr_chairs;     // pointer in shm for number of chairs
static int      *ptr_dream;      // pointer in shm for checking dream
static int      *ptr_queueLen;   // pointer in shm for lenght of queue
static int      *ptr_semID;      // pointer in shm for sem id
static int      *ptr_invitedID;   // pointer in shm for invited client ID
static int      *ptr_ifOnChair;   // pointer in shm with inf. that invited client is on chair or not

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

void get_fifo()
{

    int shmid;
    key_t key;

    key = ftok(ptr_p, QUEUE_PROJ_ID);

    shmid = shmget(key, sizeof(int), S_IRUSR | S_IWUSR);
    if(shmid == -1)
    {
        perror("get_fifo -> shmget");
        exit(EXIT_FAILURE);
    }

    ptr_f = shmat(shmid, NULL, 0);
    if(ptr_f == (int *) -1)
    {
        perror("get_fifo -> shmat");
        exit(EXIT_FAILURE);
    }

}

void get_fifo_client()
{
    int shmid;
    key_t key;

    key = ftok(ptr_p, QUEUE_CLIENT_PROJ_ID);

    shmid = shmget(key, sizeof(int), S_IRUSR | S_IWUSR);
    if(shmid == -1)
    {
        perror("get_fifo_client -> shmget");
        exit(EXIT_FAILURE);
    }

    ptr_f_client = shmat(shmid, NULL, 0);
    if(ptr_f_client == (int *) -1)
    {
        perror("get_fifo_client -> shmat");
        exit(EXIT_FAILURE);
    }

}

void get_chairs()
{

    int shmid;
    key_t key;

    key = ftok(ptr_p, CHAIRS_PROJ_ID);

    shmid = shmget(key, sizeof(int), S_IRUSR | S_IWUSR);
    if(shmid == -1)
    {
        perror("get_chairs -> shmget");
        exit(EXIT_FAILURE);
    }

    ptr_chairs = shmat(shmid, NULL, 0);
    if(ptr_chairs == (int *) -1)
    {
        perror("get_chairs -> shmat");
        exit(EXIT_FAILURE);
    }

}

void get_dream()
{

    int shmid;
    key_t key;

    key = ftok(ptr_p, DREAM_PROJ_ID);

    shmid = shmget(key, sizeof(int), S_IRUSR | S_IWUSR);
    if(shmid == -1)
    {
        perror("get_dream -> shmget");
        exit(EXIT_FAILURE);
    }

    ptr_dream = shmat(shmid, NULL, 0);
    if(ptr_dream == (int *) -1)
    {
        perror("get_dream -> shmat");
        exit(EXIT_FAILURE);
    }

}

void get_queueLen()
{

    int shmid;
    key_t key;

    key = ftok(ptr_p, QUEUELEN_PROJ_ID);

    shmid = shmget(key, sizeof(int), S_IRUSR | S_IWUSR);
    if(shmid == -1)
    {
        perror("get_queueLen -> shmget");
        exit(EXIT_FAILURE);
    }

    ptr_queueLen = shmat(shmid, NULL, 0);
    if(ptr_queueLen == (int *) -1)
    {
        perror("get_fifo -> shmat");
        exit(EXIT_FAILURE);
    }

}

void get_semID()
{

    int shmid;
    key_t key;

    key = ftok(ptr_p, SEMID_PROJ_ID);

    shmid = shmget(key, sizeof(int), S_IRUSR | S_IWUSR);
    if(shmid == -1)
    {
        perror("get_semID -> shmget");
        exit(EXIT_FAILURE);
    }

    ptr_semID = shmat(shmid, NULL, 0);
    if(ptr_semID == (int *) -1)
    {
        perror("get_semID -> shmat");
        exit(EXIT_FAILURE);
    }

}


void get_invitedID()
{

    int shmid;
    key_t key;

    key = ftok(ptr_p, INVITEDID_PROJ_ID);

    shmid = shmget(key, sizeof(int), S_IRUSR | S_IWUSR);
    if(shmid == -1)
    {
        perror("get_invitedID -> shmget");
        exit(EXIT_FAILURE);
    }

    ptr_invitedID = shmat(shmid, NULL, 0);
    if(ptr_invitedID == (int *) -1)
    {
        perror("ptr_invitedID -> shmat");
        exit(EXIT_FAILURE);
    }

}


void get_ifOnChair()
{

    int shmid;
    key_t key;

    key = ftok(ptr_p, IFONCHAR_PROJ_ID);

    shmid = shmget(key, sizeof(int), S_IRUSR | S_IWUSR);
    if(shmid == -1)
    {
        perror("get_ifOnChair -> shmget");
        exit(EXIT_FAILURE);
    }

    ptr_ifOnChair = shmat(shmid, NULL, 0);
    if(ptr_semID == (int *) -1)
    {
        perror("get_ifOnChair -> shmat");
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

void get_shm_variables()
{
    get_pathname();
    get_fifo();
    get_fifo_client();
    get_chairs();
    get_dream();
    get_queueLen();
    get_semID();
    get_invitedID();
    get_ifOnChair();
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
    printf("Opuszczenie zakładku z powodu braku wolnych miejsc w poczekalni: %d [%ld:%ld]\n", getpid(), time_info.tv_sec/60, time_info.tv_nsec/1000);
    give_semaphore(SEM_Q);
    exit(EXIT_SUCCESS);
}

void go_out_end_cutting()
{
    time_point();
    printf("Opuszczenie zakładu po zakończeniu strzyżenia: %d [%ld:%ld]\n", getpid(), time_info.tv_sec/60, time_info.tv_nsec/1000);
    *ptr_ifOnChair = 0;
}

void wait_in_queue()
{

    time_point();
    printf("Zajęcie miejsca w poczekalni: %d [%ld:%ld]\n", getpid(), time_info.tv_sec/60, time_info.tv_nsec/1000);

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
    printf("Obudzenie Golibrody: %d [%ld:%ld]\n", getpid(), time_info.tv_sec/60, time_info.tv_nsec/1000);
    *ptr_invitedID = getpid();
    *ptr_dream = 0;

    give_semaphore(SEM_FAS);
    give_semaphore(SEM_Q);



}

void sit_in_chair()
{
    time_point();
    printf("Zjęcie miejsca na krześle do strzyżenia: %d [%ld:%ld]\n", getpid(), time_info.tv_sec/60, time_info.tv_nsec/1000);
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
