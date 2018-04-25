#include <stdbool.h>
#include <mqueue.h>
#include <fcntl.h>
#define serverNmb 106 // Number used to create server key
#define clientNmb 50 // Number used to create client key
#define maxClientsNmb 2
#define stringLenght 50 // Includint null byte ('\0')
#define clQueueNameLenght 5
#define argNmb 2 // max number of argumenst including task in file tasks.txt
#define msg_prio 1
#define msg_prio_stop 2

#define CL_QUEUENAME 1
#define SR_CLIENTID 2
#define CL_CLIENTSTP 3
#define SR_CNTHANDLE 4
#define SR_CLOSED_Q 5

#define CL_SR_MIRROR 10
#define CL_SR_CALC 11
#define CL_SR_TIME 12
#define CL_SR_END 13
#define CL_SR_CLOSED_Q 14


#define GRN   "\x1B[32m"
#define RESET "\x1B[0m"
#define RED   "\x1B[31m"

#define characters "abcdefghijklmnoprstqwxyzABCDEFGHIJKLMNOPRSTQWXYZ"

void open_client_queue(void);
void send_clientID_to_client(void);
void send_cant_handle(void);

// **** CLIENT FUNCTION SIGNATURES **** //
void delete_queue(void);
void send_stop_signal_to_server(void);
int receive_message_from_server(void);


struct ClientInfo
{
    char clQueueName[clQueueNameLenght];
    int qID;
    int clientID;
    bool used;

} ClientInfo;

struct ClientMessage
{
    long mType;
    int clientID;
    char clQueueName[clQueueNameLenght];
    pid_t pid;
    char string[stringLenght];
    int a, b;
    int calcType; // 0 - + 1 - - 2 - * 3 - /
    double result;
    char timeString[100];

} ClientMessage; 


#define srQueueName "/serverQueue"