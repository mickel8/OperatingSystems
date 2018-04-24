#include <stdbool.h>
#define serverNmb 106 // Number used to create server key
#define clientNmb 50 // Number used to create client key
#define maxClientsNmb 5
#define stringLenght 50 // Includint null byte ('\0')
#define argNmb 2 // max number of argumenst including task in file tasks.txt

#define CL_QUEUEKEY 1
#define SR_CLIENTID 2
#define CL_CLIENTSTP 3

#define CL_SR_MIRROR 10
#define CL_SR_CALC 11
#define CL_SR_TIME 12
#define CL_SR_END 13


#define GRN   "\x1B[32m"
#define RESET "\x1B[0m"
#define RED   "\x1B[31m"

void open_client_queue(void);
void send_clientID_to_client(void);


// **** CLIENT FUNCTION SIGNATURES **** //
void delete_queue(void);
void send_stop_signal_to_server(void);


struct ClientInfo
{
    key_t key;
    int qID;
    int clientID;
    bool used;

} ClientInfo;

struct ClientMessage
{
    long mType;
    int clientID;
    key_t qKey;
    pid_t pid;
    char string[stringLenght];
    int a, b;
    int calcType; // 0 - + 1 - - 2 - * 3 - /
    double result;
    char timeString[100];

} ClientMessage; 

struct clientIDMessage
{
    long mType;
    int clientID;

} clientIDMessage;

