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
static char lastClQueueName[clQueueNameLenght];
static int clientsNmb = 0; 
static int previousIndexAvailable = -1; // -1 means no >= 0 yes and points to specified index
static int lastClIndex;
static int messagesInQueue = -1;
static int endMessageFlag = 0;
static struct ClientInfo clients[maxClientsNmb];



// Create new message queue
void create_queue()
{

    struct mq_attr attr;  
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;  
    attr.mq_msgsize = sizeof(struct ClientMessage);  
    attr.mq_curmsgs = 0; 

    srQueueID = mq_open(srQueueName, O_RDWR | O_CREAT | O_EXCL,  S_IRUSR | S_IWUSR, &attr);
    if(srQueueID == -1)
    {
        perror("create_queue() -> mq_open()");
        exit(EXIT_FAILURE);
    }
}

// Send message to client
void send_message_to_client(struct ClientMessage response)
{
    int res;
    
    res = mq_send(clients[response.clientID].qID, (const char *)&response, sizeof(response), msg_prio);
    if(res == -1)
    {
        perror("send_clQueueKey_to_server() -> msgsnd()");
        exit(EXIT_FAILURE);
    }
}

// Close and delete message queue. Close all opened queues.
void close_and_delete_queue()
{
    int res;
    struct ClientMessage response;


    res = mq_close(srQueueID);
    if(res == -1)
    {
        perror("close_and_delete_queue() -> mq_close1()");
        exit(EXIT_FAILURE);
    }

    res = mq_unlink(srQueueName);
    if(res == -1)
    {
        perror("close_and_delete_queue() -> mq_unlink()");
        exit(EXIT_FAILURE);
    }


    for(int i = 0; i < maxClientsNmb; i++)
    {
        if(clients[i].used)
        {
            response.mType = SR_CLOSED_Q;
            response.clientID = i;
            send_message_to_client(response);
            res = mq_close(clients[i].qID);
            if(res == -1)
            {
                perror("close_and_delete_queue() -> mq_close()2");
                exit(EXIT_FAILURE);
            }
            else
            {
                printf("The queue %d has been closed\n", clients[i].clientID);
            }
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

// Recive message from client
int receive_message_from_client()
{
    struct ClientMessage request;
    if(mq_receive(srQueueID, (char *)&request, sizeof(request) + 1, NULL) == -1)
    {
        perror("receive_message_from_client() -> msgrcv()");
        exit(EXIT_FAILURE);
    }

    int type = request.mType;
    
    char * revers;

    struct ClientMessage response;
    response.clientID = request.clientID;
    response.pid = request.pid;
    

    switch(type)
    {
        case CL_QUEUENAME:
            // Max clients number has been reached
            if(clientsNmb == maxClientsNmb)
            {
                printf("Cannot handle more clients. Max number of clients has been reached(%d)\n", maxClientsNmb);
                strcpy(lastClQueueName, request.clQueueName);
                send_cant_handle();
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
                        strcpy(clients[i].clQueueName, request.clQueueName);
                        clients[i].clientID = i;
                        lastClIndex = i;
                        strcpy(lastClQueueName, request.clQueueName);
                        open_client_queue();
                        send_clientID_to_client();
                        printf("New client: %s\n", clients[lastClIndex].clQueueName);
                        break;
                    }
                }  
            }
            break;
        case CL_CLIENTSTP:
            remove_client(request);
            response.mType = CL_SR_CLOSED_Q;
            send_message_to_client(response);
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
            struct mq_attr buf;
            if(mq_getattr(srQueueID, &buf) == -1)
            {
                perror("receive_message_from_clien() -> msgctl()");
                close_and_delete_queue();
                exit(EXIT_FAILURE);
            }
            messagesInQueue = buf.mq_curmsgs;
            endMessageFlag = 1;
            printf("Messages in queue %d\n", messagesInQueue);
            break;
        default:
            printf("Received unknown signal. Signal number: %d\n", type);
            break;
    }

    return 0;
}

void send_cant_handle()
{
    int res;
    int queue;

    struct ClientMessage message;
    message.mType = SR_CNTHANDLE;
    message.clientID = -1;
    
    queue = mq_open( lastClQueueName, O_WRONLY);
    if( queue == -1 )
    {
        perror("create_queue() -> msgget()");
        exit(-1);
    }

    res = mq_send(queue, (const char *)&message, sizeof(message), msg_prio);
    if(res == -1)
    {
        perror("send_clQueueID_to_server() -> mq_send()");
        exit(EXIT_FAILURE);
    }

    res = mq_close(queue);
    if(res == -1)
    {
        perror("close_and_delete_queue() -> mq_close()");
        exit(EXIT_FAILURE);
    }


}

// Send client ID to client
void send_clientID_to_client()
{
    int res;
    
    struct ClientMessage message;
    message.mType = SR_CLIENTID;
    message.clientID = lastClIndex;

    res = mq_send(clients[lastClIndex].qID, (const char *)&message, sizeof(message), msg_prio);
    if(res == -1)
    {
        perror("send_clQueueID_to_server() -> mq_send()");
        exit(EXIT_FAILURE);
    }
}


// Open client message queue
void open_client_queue()
{

    int queue;
    
    queue = mq_open( lastClQueueName, O_WRONLY);
    if( queue == -1 )
    {
        perror("create_queue() -> msgget()");
        exit(-1);
    }

    clients[lastClIndex].qID = queue;
    
}


int main(int argc, char **argv)
{
    if(atexit(close_and_delete_queue) != 0)
    {
        perror("main() -> atexit()");
        exit(EXIT_FAILURE);
    }

    notify_sigint_handling();
    create_queue();
    
    
    while(1)
    {
        if(endMessageFlag == 1)
        {
            printf("Messages to end %d\n", messagesInQueue);
            if(messagesInQueue <= 0)
            {
                struct mq_attr attr;
                if(mq_getattr(srQueueID, &attr) == -1)
                {
                    perror("receive_message_from_clien() -> msgctl()");
                    close_and_delete_queue();
                    exit(EXIT_FAILURE);
                }
                messagesInQueue = attr.mq_curmsgs;
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