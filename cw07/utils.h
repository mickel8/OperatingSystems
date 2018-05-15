#include <stdlib.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define keyForPathname 120321

#define PID_PROJ_ID 110
#define QUEUE_PROJ_ID 132
#define CHAIRS_PROJ_ID 112
#define DREAM_PROJ_ID 113
#define QUEUELEN_PROJ_ID 114
#define SEM_PROJ_ID 140
#define QUEUE_CLIENT_PROJ_ID 133
#define SEMID_PROJ_ID 117
#define INVITEDID_PROJ_ID 118
#define IFONCHAR_PROJ_ID 119

#define SEM_FAS 0   // 0
#define SEM_D 1     // 1
#define SEM_Q 2     // 1
#define SEM_C 3     // 0


#define SEM_NMB 4

#define THE_ONLY_ONE 0
#define FROM_QUEUE 1

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};

struct msgp
{
    long mtype;
    pid_t pid;
};
