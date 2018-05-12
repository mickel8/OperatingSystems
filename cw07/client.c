#include <stdio.h>

#include "utils.h"

static char *ptr_p; // pointer for pathname for ftok()
// static int  *ptr_f; // pointer for fifo queue id
// static int queueID;

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

    printf("Pathname: %s\n", ptr_p);

}


int main(int argc, char **argv)
{

    int pid = getpid();
    int nmbOfHaircut = atoi(argv[1]);

    get_pathname();


    
    return 0;
}
