#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <errno.h>

#include "utils.h"


static char *homedir;
static int srQueueID;
static int lastClQueueKey;
static int clientsKeysNmb = 0;
static int clientsKeys[maxClientsNmb];


// Assign home directory path to homeDir variable
void assign_homedir_env()
{
    homedir = getenv("HOME");
    if ( homedir == NULL )
    {
        printf("Couldn't find HOME environment variable\n");
        exit(-1);
    }
}

// Create new message queue
void create_queue()
{
    key_t key;
    int queue;

    key = ftok( homedir, serverNmb );
    if ( key == -1 )
    {
        perror( "create_queue() -> ftok()");
        exit(-1);
    }
    
    queue = msgget( key, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if( queue == -1 )
    {
        perror("create_queue() -> msgget()");
        exit(-1);
    }
    
    srQueueID = queue;
}

// Delete message queue
void delete_queue()
{
    int res;
    res = msgctl(srQueueID, IPC_RMID, NULL);

    if( res == -1 )
    {
        perror("delete_queue() -> msgctl()");
        exit(EXIT_FAILURE);
    }
}

// Behaviour specified in case of receiving SIGINT signal
void sigint_handler()
{
    delete_queue();
    exit(EXIT_SUCCESS);
}

// Notify about handling SIGINT signal
void notify_sigint_handling()
{
    struct sigaction act;
    
    act.sa_handler = sigint_handler;
    act.sa_flags   = 0;
    if(sigfillset(&act.sa_mask) == -1)
    {
        perror("notify_sigin_handling() -> sigfillset()");
        exit(EXIT_FAILURE);
    }

    if(sigaction(SIGINT, &act, NULL) == -1)
    {
        perror("notify_sigint_handling() -> sigaction()");
        exit(EXIT_FAILURE);
    }

}

// Recive clQueueID from client
int recive_clQueueID_from_client()
{
    struct queueKeyMessage message;
    if(msgrcv(srQueueID, &message, sizeof(message) - sizeof(long), CL_QUEUEID, IPC_NOWAIT) == -1)
    {
        if(errno != ENOMSG)
        {
            perror("recive_clQueueID_from_client() -> msgrcv()");
            exit(EXIT_FAILURE);
        }
        else
        {
            return -1;
        }
    }

    if(clientsKeysNmb == maxClientsNmb)
    {
        printf("Cannot handle more clients. Max number of clients has been reached(%d)\n", maxClientsNmb);
        return -1;
    }

    clientsKeys[clientsKeysNmb] = message.qKey;
    lastClQueueKey = message.qKey;
    clientsKeysNmb++;

    printf("clQueueID: %d\n", message.qKey);

    return 0;
}

// Open client message queue
void open_client_queue()
{
    key_t key;
    int queue;

    key = ftok( homedir, serverNmb );
    if ( key == -1 )
    {
        perror( "create_queue() -> ftok()");
        exit(-1);
    }

    queue = msgget( key, 0);
    if( queue == -1 )
    {
        perror("create_queue() -> msgget()");
        exit(-1);
    }

    srQueueID = queue;
}


int main(int argc, char **argv)
{
    if(atexit(delete_queue) != 0)
    {
        perror("main() -> atexit()");
        exit(EXIT_FAILURE);
    }

    notify_sigint_handling();
    assign_homedir_env();
    create_queue();
    
    
    while(1)
    {
        if(recive_clQueueID_from_client() == 0)
        {
            open_client_queue();
        }

        sleep(5);
    }

    return 0;
}