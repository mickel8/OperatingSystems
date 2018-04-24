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
#include <string.h>
#include <time.h>

#include "utils.h"


static char *homedir;
static int srQueueID;
static int lastClQueueKey;
static int clientsNmb = 0; 
static int previousIndexAvailable = -1; // -1 means no >= 0 yes and points to specified index
static int lastClIndex;
static int messagesInQueue = -1;
static int endMessageFlag = 0;
static struct ClientInfo clients[maxClientsNmb];


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
        if(errno == EIDRM)
        {
            printf("Queue has been already deleted\n");
            exit(EXIT_SUCCESS);
        }
        else
        {
            perror("delete_queue() -> msgctl()");
            exit(EXIT_FAILURE);
        }
    }
}

// Behaviour specified in case of receiving SIGINT signal
void sigint_handler()
{
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

// Remove client info from array
void remove_client(struct ClientMessage message)
{
    printf("Received stop signal from client %d client ID: %d\n", message.pid, message.clientID);
    clientsNmb--;
    clients[message.clientID].used = false;
}

// Mirror function
char *mirror(char *string)
{
    int i = 0;
    int j = strlen(string) - 1;
    char *revers = calloc(stringLenght, sizeof(char));
    
    while(string[i] != '\0')
    {
        revers[j] = string[i];
        j--;
        i++;
    }

    revers[strlen(string)] = '\0';

    return revers;
}

// Time function
void get_time(struct ClientMessage *response)
{
    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    char output[21];
    sprintf(output, "[%d %d %d %d:%d:%d]",timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    strcpy(response -> timeString, output);
}


// Send message to client
void send_message_to_client(struct ClientMessage response)
{
    int res;
    
    res = msgsnd(clients[response.clientID].qID, &response, sizeof(response) - sizeof(long), IPC_NOWAIT);
    if(res == -1)
    {
        perror("send_clQueueKey_to_server() -> msgsnd()");
        exit(EXIT_FAILURE);
    }
}

// Recive message from client
int receive_message_from_client()
{
    struct ClientMessage request;
    if(msgrcv(srQueueID, &request, sizeof(request) - sizeof(long), 0, IPC_NOWAIT) == -1)
    {
        if(errno != ENOMSG)
        {
            perror("receive_message_from_client() -> msgrcv()");
            exit(EXIT_FAILURE);
        }
        else
        {
            return -1;
        }
    }

    int type = request.mType;
    
    char * revers;

    struct ClientMessage response;
    response.clientID = request.clientID;
    response.pid = request.pid;
    

    switch(type)
    {
        case CL_QUEUEKEY:
            // Max clients number has been reached
            if(clientsNmb == maxClientsNmb)
            {
                printf("Cannot handle more clients. Max number of clients has been reached(%d)\n", maxClientsNmb);
                return -1;
            }
            // New client can be handle
            else
            {
                for(int i = 0; i < maxClientsNmb; i++)
                {
                    if(!clients[i].used)
                    {
                        clientsNmb++;
                        clients[i].used = true;
                        clients[i].key = request.qKey;
                        clients[i].clientID = i;
                        lastClIndex = i;
                        lastClQueueKey = request.qKey;
                        open_client_queue();
                        send_clientID_to_client();
                        printf("New client: %d\n", clients[lastClIndex].key);
                        break;
                    }
                }  
            }
            break;
        case CL_CLIENTSTP:
            remove_client(request);
            break;
        case CL_SR_MIRROR:
            printf("Receive MIRROR task from:\nPid: %d\nClientID: %d\n", request.pid, request.clientID);
            revers = mirror(request.string);    
            response.mType = CL_SR_MIRROR;
            strcpy(response.string, revers);
            send_message_to_client(response);
            break;
        case CL_SR_CALC:
            printf("Receive CALC task from:\nPid: %d\nClientID: %d\n", request.pid, request.clientID);
            switch(request.calcType)
            {
                case 0:
                    response.result = (double) request.a + request.b;
                    break;
                case 1:
                    response.result = (double) request.a - request.b;
                    break;
                case 2:
                    response.result = (double) request.a * request.b;
                    break;
                case 3:
                    response.result = (double) request.a / request.b;
                    break;
                default:
                    printf("Unknown operator\n");
                    break;

            }
            response.mType = CL_SR_CALC;
            send_message_to_client(response);
            break;
        case CL_SR_TIME:
            printf("Receive TIME task from:\nPid: %d\nClientID: %d\n", request.pid, request.clientID);
            get_time(&response);
            response.mType = CL_SR_TIME;
            send_message_to_client(response);
            break;
        case CL_SR_END:
            printf("Receive END task from:\nPid: %d\nClientID: %d\n", request.pid, request.clientID);
            struct msqid_ds buf;
            if(msgctl(srQueueID, IPC_STAT, &buf) == -1)
            {
                perror("receive_message_from_clien() -> msgctl()");
                delete_queue();
                exit(EXIT_FAILURE);
            }
            messagesInQueue = buf.msg_qnum;
            endMessageFlag = 1;
            printf("Messages in queue %d\n", messagesInQueue);
            break;
        default:
            printf("Received unknown signal. Signal number: %d\n", type);
            break;
    }

    errno = 0;
    return 0;
}

// Send client ID to client
void send_clientID_to_client()
{
    int res;
    
    struct clientIDMessage message;
    message.mType = SR_CLIENTID;
    message.clientID = lastClIndex;

    res = msgsnd(clients[lastClIndex].qID, &message, sizeof(message) - sizeof(long), IPC_NOWAIT);
    if(res == -1)
    {
        perror("send_clQueueKey_to_server() -> msgsnd()");
        exit(EXIT_FAILURE);
    }
}


// Open client message queue
void open_client_queue()
{

    int queue;

    queue = msgget( lastClQueueKey, 0 );
    if( queue == -1 )
    {
        perror("create_queue() -> msgget()");
        exit(-1);
    }

    clients[lastClIndex].qID = queue;
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
        if(endMessageFlag == 1)
        {
            printf("Messages to end %d\n", messagesInQueue);
            if(messagesInQueue <= 0)
            {
                struct msqid_ds buf;
                if(msgctl(srQueueID, IPC_STAT, &buf) == -1)
                {
                    perror("receive_message_from_clien() -> msgctl()");
                    delete_queue();
                    exit(EXIT_FAILURE);
                }
                messagesInQueue = buf.msg_qnum;
                printf("Messages in queue %d\n", messagesInQueue);
                exit(EXIT_SUCCESS);
            }
            else
            {
                receive_message_from_client();
                messagesInQueue--;
            }
        }
        else
        {
            receive_message_from_client();
        }
        
        sleep(2);
    }

    return 0;
}