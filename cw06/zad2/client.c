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
static int clQueueID;
static int srQueueID;
static int clientID;
static pid_t pid;
static key_t clKey;
static char *clQueueName;


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

// Random queue name
char *random_name()
{
    char *ptr = calloc(clQueueNameLenght, sizeof(char));
    ptr[0] = '/';
    for(int i = 1; i < clQueueNameLenght; i++)
    {
        ptr[i] = characters[rand()*1000%48];

    }

    return ptr;
}

// Create new message queue
void create_queue()
{
    struct mq_attr attr;  
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;  
    attr.mq_msgsize = sizeof(struct ClientMessage);  
    attr.mq_curmsgs = 0; 

    clQueueName = random_name();
    clQueueID = mq_open(clQueueName, O_RDWR | O_CREAT | O_EXCL,  S_IRUSR | S_IWUSR, &attr);
    if(clQueueID == -1)
    {
        perror("create_queue() -> mq_open()");
        exit(EXIT_FAILURE);
    }    
}

// Close and delete message queue
void close_and_delete_queue()
{
    
    send_stop_signal_to_server();
    receive_message_from_server();

    int res;

    res = mq_close(clQueueID);
    if(res == -1)
    {
        perror("close_and_delete_queue() -> mq_close()");
        exit(EXIT_FAILURE);
    }

    res = mq_unlink(clQueueName);
    if(res == -1)
    {
        perror("close_and_delete_queue() -> mq_unlink()");
        exit(EXIT_FAILURE);
    }
    
}

// Open server message queue
void open_server_queue()
{
    srQueueID = mq_open(srQueueName, O_WRONLY);
    if(srQueueID == -1)
    {
        perror("open_server_queue() -> mq_open()");
        exit(EXIT_FAILURE);
    }   
    printf("server id: %d\n", srQueueID); 
}

// Send client message queue ID to server
void send_clQueueName_to_server()
{
    int res;
    
    struct ClientMessage message;
    message.mType = CL_QUEUENAME;
    strcpy(message.clQueueName, clQueueName);

    res = mq_send(srQueueID, (const char *)&message, sizeof(message), msg_prio);
    if(res == -1)
    {
        perror("send_clQueueName_to_server() -> msgsnd()");
        exit(EXIT_FAILURE);
    }
}

// Send stop signal to server that informs server that client was exited
void send_stop_signal_to_server()
{
    int res;
    
    struct ClientMessage message;
    message.mType = CL_CLIENTSTP;
    message.clientID = clientID;
    message.pid = pid;

    res = mq_send(srQueueID, (const char *)&message, sizeof(message), msg_prio_stop);
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
    struct ClientMessage message;
    if(mq_receive(clQueueID, (char *)&message, sizeof(message)+1, NULL) == -1)
    {
        perror("recive_clQueueID_from_client() -> msgrcv()");
        exit(EXIT_FAILURE);
    }

    if(message.clientID == -1)
    {
        printf("Server cannot handle more clients\n");
        _exit(EXIT_FAILURE);
    }

    clientID = message.clientID;
    printf("My client ID: %d\n", clientID);

    return 0;
}

// Send message to server
void send_message_to_server(struct ClientMessage request)
{   
    int res;
   
    res = mq_send(srQueueID, (const char *)&request, sizeof(request), msg_prio);
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
    if(mq_receive(clQueueID, (char *)&respond, sizeof(respond)+1, NULL) == -1)
    {
        perror("receive_message_from_client() -> msgrcv()");
        exit(EXIT_FAILURE);
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
        case CL_SR_CLOSED_Q:
            printf("Queue was closed on server\n");
            break;
        case SR_CLOSED_Q:
            printf("Server is going shut down. Queue was closed on server\n");
            _exit(1);
        default:
            printf(RED "Received unknown type message. Type number: %d\n" RESET, type);
            break;
    }

    return 0;
}



int main(int argc, char **argv)
{

    srand(time(NULL));

    if(atexit(close_and_delete_queue) != 0)
    {
        perror("main() -> atexit()");
        exit(EXIT_FAILURE);
    }

    pid = getpid();
    printf("My pid %d\n", pid);


    notify_sigint_handling();
    create_queue();
    open_server_queue();
    send_clQueueName_to_server();
    
    receive_queueID_given_by_server();

    // Preaparing file
    FILE *file = fopen("./tasks.txt", "r");
    if(file == NULL)
    {
        perror("main() -> fopen()");
        send_stop_signal_to_server();
        receive_message_from_server();
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

            send_stop_signal_to_server();
            receive_message_from_server();

            printf("Sending END task to server\n");
            send_message_to_server(request);
            _exit(EXIT_SUCCESS);
        }
        else 
        {
            printf("Unknown task %s\n", args[0]);
            break;
        }

        receive_message_from_server();

    }

    if(errno)
    {
        perror("main() -> getline()");
        send_stop_signal_to_server();
        receive_message_from_server();
        exit(EXIT_FAILURE);
    }

    while(1);
        
    return 0;
}