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
static int clQueueID;
static int srQueueID;
static int clientID;
static pid_t pid;
static key_t clKey;




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
    int i = 1;

    key = ftok( homedir, clientNmb );
    if ( key == -1 )
    {
        perror( "create_queue() -> ftok()");
        exit(-1);
    }
    
    queue = msgget( key, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    // while we cannot create new messeage queue for client due to existance of queue of given key
    while(queue == -1 && errno == EEXIST && i < maxClientsNmb)
    {
        // we are creating new key
        key = ftok( homedir, clientNmb + i );
        if ( key == -1 )
        {
            perror( "create_queue() -> ftok()");
            exit(-1);
        }

        // and attempting to create queue once again
        queue = msgget( key, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
        i++;
    }

    // if queue is still -1 that means either there was error or max clients number has been reached
    if( queue == -1 )
    {
        if(i == maxClientsNmb)
        {
            printf("Cannot create more clients\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            perror("create_queue() -> msgget()");
            exit(-1);      
        }
    }
    
    clQueueID = queue;
    clKey = key;
}

// Delete message queue
void delete_queue()
{
    int res;
    res = msgctl(clQueueID, IPC_RMID, NULL);

    if( res == -1 )
    {
        perror("delete_queue() -> msgctl()");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

// Open server message queue
void open_server_queue()
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

// Send client message queue ID to server
void send_clQueueKey_to_server()
{
    int res;
    
    struct ClientMessage message;
    message.mType = CL_QUEUEKEY;
    message.qKey = clKey;

    res = msgsnd(srQueueID, &message, sizeof(message) - sizeof(long), IPC_NOWAIT);
    if(res == -1)
    {
        perror("send_clQueueKey_to_server() -> msgsnd()");
        exit(EXIT_FAILURE);
    }
}

// Send stop signal to server
void send_stop_signal_to_server()
{
    int res;
    
    struct ClientMessage message;
    message.mType = CL_CLIENTSTP;
    message.clientID = clientID;
    message.pid = pid;

    res = msgsnd(srQueueID, &message, sizeof(message) - sizeof(long), IPC_NOWAIT);
    if(res == -1)
    {
        perror("send_clQueueKey_to_server() -> msgsnd()");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("sent stop signal to server, my pid: %d, my client ID: %d\n", pid, clientID);
    }   
}

// Behaviour specified in case of receiving SIGINT signal
void sigint_handler()
{
    send_stop_signal_to_server();
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

// Receive message queue ID given by server
int receive_queueID_given_by_server()
{
    struct clientIDMessage message;
    if(msgrcv(clQueueID, &message, sizeof(message) - sizeof(long), SR_CLIENTID, IPC_NOWAIT) == -1)
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

    clientID = message.clientID;
    printf("My client ID: %d\n", clientID);
    return 0;
}

int main(int argc, char **argv)
{
    if(atexit(delete_queue) != 0)
    {
        perror("main() -> atexit()");
        exit(EXIT_FAILURE);
    }
    
    pid = getpid();

    notify_sigint_handling();
    assign_homedir_env();
    create_queue();
    open_server_queue();
    send_clQueueKey_to_server();
    
    while(receive_queueID_given_by_server() == -1);
    {
    }
    
    while(1)
    {

    }
    return 0;
}