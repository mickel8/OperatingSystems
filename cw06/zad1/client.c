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

#include "utils.h"


static char *homedir;
static int clQueueID;
static int srQueueID;
static int clientID;
static pid_t pid;
static key_t clKey;



// **** PRINTING FUNCTIONS **** //
void print_mirror_response(struct ClientMessage response)
{
    printf("Response for" RED " MIRROR " RESET "request\nPid: %d\nClientID: %d\n" GRN "Reversed string: %s\n" RESET, response.pid, response.clientID, response.string);
}
void print_calc_response(struct ClientMessage response)
{
    printf("Response for" RED " CALC " RESET "request\nPid: %d\nClientID: %d\n" GRN "Result: %f\n" RESET, response.pid, response.clientID, response.result);
}
void print_time_response(struct ClientMessage response)
{
    printf("Response for" RED " TIME " RESET "request\nPid: %d\nClientID: %d\n" GRN "Time: %s\n" RESET, response.pid, response.clientID, response.timeString);    
}
// *********** END ************ //




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

// Split arguments including function task
char **split_args(char *line){
    
    char **args = (char **)calloc(argNmb, sizeof(char *));
    args[0] = strtok(line, " ");
    if(args[0][strlen(args[0]) - 1] == '\n') args[0][strlen(args[0]) - 1] = '\0'; // for TIME and END TASKS because of \n at the end
    args[1] = strtok(NULL, "\n");
           
    return args;
} 

// Behaviour specified in case of receiving SIGINT signal
void sigint_handler()
{
    send_stop_signal_to_server();
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
    errno = 0; // clean previous ENOMSG error
    printf("My client ID: %d\n", clientID);
    return 0;
}

// Send message to server
void send_message_to_server(struct ClientMessage request)
{   
    int res;
  

    res = msgsnd(srQueueID, &request, sizeof(request) - sizeof(long), IPC_NOWAIT);
    if(res == -1)
    {
        perror("send_clQueueKey_to_server() -> msgsnd()");
        exit(EXIT_FAILURE);
    } 
}

// Receive message from server
int receive_message_from_server()
{
    struct ClientMessage respond;
    if(msgrcv(clQueueID, &respond, sizeof(respond) - sizeof(long) + stringLenght * sizeof(char), 0, IPC_NOWAIT) == -1)
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

    int type = respond.mType;

    switch(type)
    {
        case CL_SR_MIRROR:
            print_mirror_response(respond);
            break;
        case CL_SR_CALC:
            print_calc_response(respond);
            break;
        case CL_SR_TIME:
            print_time_response(respond);
            break;
        default:
            printf(RED "Received unknown type message. Type number: %d\n" RESET, type);
            break;
    }


    errno = 0;
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

    printf("My pid %d\n", pid);


    notify_sigint_handling();
    assign_homedir_env();
    create_queue();
    open_server_queue();
    send_clQueueKey_to_server();
    
    while(receive_queueID_given_by_server() == -1);
   
    
    // Preaparing file
    FILE *file = fopen("./tasks.txt", "r");
    if(file == NULL)
    {
        perror("main() -> fopen()");
        send_stop_signal_to_server();
        exit(EXIT_FAILURE);
    }

    int a, b; // for numbers for CALC TASK
    char *line = NULL; // for getline
    long unsigned int n = 0; // for getline
    char **args; // to split arguments

    struct ClientMessage request;
    request.clientID = clientID;
    request.pid = pid;

    while(getline(&line, &n, file) != -1)
    {

        args = split_args(line);


        if(strcmp(args[0], "MIRROR") == 0)
        {
            char *string = args[1];
            if(strlen(string) >= stringLenght)
            {
                printf("Too big string given to MIRROR task\n");
                break;
            }

            request.mType = CL_SR_MIRROR;
            strcpy(request.string, args[1]);

            printf("Sending MIRROR task to server\n");
            send_message_to_server(request);
            
        }
        else if(strcmp(args[0], "CALC") == 0)
        {
            // args[1][1] shoul be operator +-*/
            int calcType = -1;
            if(args[1][1] == '+') calcType = 0;
            else if(args[1][1] == '-') calcType = 1;
            else if(args[1][1] == '*') calcType = 2;
            else if(args[1][1] == '/') calcType = 3;
            else printf("Bad operator in CALC task\n");
            if(calcType == -1);
            else
            {
                request.mType = CL_SR_CALC;
                request.a = args[1][0] - 48;
                request.calcType = calcType;
                request.b = args[1][2] - 48;
                printf("Sending CALC task to server\n");
                send_message_to_server(request);
            }
        }
        else if(strcmp(args[0], "TIME") == 0)
        {
            request.mType = CL_SR_TIME;
            printf("Sending TIME task to server\n");
            send_message_to_server(request);
        }
        else if(strcmp(args[0], "END") == 0)
        {
            request.mType = CL_SR_END;
            printf("Sending END task to server\n");
            send_message_to_server(request);
            exit(EXIT_SUCCESS);
        }
        else 
        {
            printf("Unknown task %s\n", args[0]);
            break;
        }

        while(receive_message_from_server() == -1);

    }

    if(errno)
    {
        perror("main() -> getline()");
        send_stop_signal_to_server();
        exit(EXIT_FAILURE);
    }

    while(1);
        
    return 0;
}