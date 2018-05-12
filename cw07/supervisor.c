#include <stdio.h>

#include "utils.h"


static char     *ptr_p;     // pointer for pathname for ftok()
static pid_t    *ptr_pid;   // pointer for golibroda's pid

int check_status(pid_t *children, int clientsNMB)
{
    int counter = 0;

    for(int i = 0; i < clientsNMB; i++)
    {
        kill(children[i], 0);
        if(errno == ESRCH)
        {
            counter++;
        }
        else if(errno)
        {
            perror("check_status -> kill");
            exit(EXIT_FAILURE);
        }

        errno = 0;
    }

    if(counter == clientsNMB) return 0;
    else return -1;
}

void get_pathname()
{
    int shmid;

    shmid = shmget(keyForPathname, 200, S_IRUSR | S_IWUSR);
    if(shmid == -1)
    {
        perror("get_pathname -> shmget");
        exit(EXIT_FAILURE);
    }

    ptr_p = shmat(shmid, NULL, 0);
    if(ptr_p == (char *) -1)
    {
        perror("get_pathname -> shmat");
        exit(EXIT_FAILURE);
    }

}

// gets pid of golibroda
void get_pid()
{
    get_pathname();

    int shmid;
    key_t key;

    key = ftok(ptr_p, PID_PROJ_ID);

    shmid = shmget(key, sizeof(pid_t), S_IRUSR | S_IWUSR);
    if(shmid == -1)
    {
        perror("get_pid -> shmget");
        exit(EXIT_FAILURE);
    }

    ptr_pid = shmat(shmid, NULL, 0);
    if(ptr_pid == (pid_t *) -1)
    {
        perror("get_pid -> shmat");
        exit(EXIT_FAILURE);
    }

    printf("Golibroda's pid: %d\n", *ptr_pid);

}



int main(int argc, char **argv)
{

    if(argc != 3)
    {
        printf("Bad number of arguments\n");
        exit(EXIT_FAILURE);
    }

    int res;
    pid_t child;
    char nmbOfHaircutS[12];
    int clientsNMB      = atoi(argv[1]);
    int nmbOfHaircut    = atoi(argv[2]);
    pid_t *children     = calloc(clientsNMB, sizeof(pid_t));

    res = sprintf(nmbOfHaircutS, "%d", nmbOfHaircut);
    if(res < 0)
    {
        perror("main -> sprintf");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < clientsNMB; i++)
    {

        child = fork();
        if(child == 0)
        {
            execl("./client", "./client", nmbOfHaircutS, NULL);

            perror("main -> execl");
            exit(EXIT_FAILURE);
        }
        else if(child > 0)
        {
            children[i] = child;
        }
        else
        {
            perror("main -> fork");
            exit(EXIT_FAILURE);
        }

    }


    get_pid();

    sleep(2);

    int status;
    for(int i = 0; i < clientsNMB; i++)
    {
        res = waitpid(children[i], &status, 0);
        if(errno)
        {
            perror("main -> waitpid");
        }
        printf("%d\n", res);
    }

    res = kill(*ptr_pid, SIGTERM);
    if(res == -1)
    {
        perror("main -> kill");
        exit(EXIT_FAILURE);
    }


    return 0;
}
