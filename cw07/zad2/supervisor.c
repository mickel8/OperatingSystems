#include <stdio.h>

#include "utils.h"

static pid_t    *ptr_pid;   // pointer for golibroda's pid

static int pid_fd;

// gets pid of golibroda
void get_pid()
{
    int res;

    res = shm_open(PID_SHM, O_RDWR, S_IWUSR | S_IRUSR);
    if(res == -1)
    {
        perror("get_pid -> shm_open");
        exit(EXIT_FAILURE);
    }

    pid_fd = res;

    ptr_pid = mmap(NULL, sizeof(pid_t), PROT_READ | PROT_WRITE, MAP_SHARED, pid_fd, 0);
    if(ptr_pid == (pid_t *)-1)
    {
        perror("get_pid -> mmap");
        exit(EXIT_FAILURE);
    }

    printf("Golibroda's pid: %d\n", *ptr_pid);

}

void clean_workplace()
{
    int res;

    res = munmap(ptr_pid, sizeof(pid_t));
    if(res == -1)
    {
        perror("clean_workplace -> munmap");
        exit(EXIT_FAILURE);
    }
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

    res = atexit(clean_workplace);
    if(res != 0)
    {
        perror("main -> atexit");
        exit(EXIT_FAILURE);
    }

    sleep(2);

    int status;
    for(int i = 0; i < clientsNMB; i++)
    {
        res = waitpid(children[i], &status, 0);
        if(errno)
        {
            perror("main -> waitpid");
        }
        //printf("Client %d has new haircut\n", res);
    }

    res = kill(*ptr_pid, SIGTERM);
    if(res == -1)
    {
        perror("main -> kill");
        exit(EXIT_FAILURE);
    }


    return 0;
}
