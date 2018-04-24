#include <stdbool.h>
#define serverNmb 11 // Number used to create server key
#define clientNmb 15 // Number used to create client key

#define CL_QUEUEKEY 1
#define SR_CLIENTID 2
#define CL_CLIENTSTP 3

#define maxClientsNmb 5

void open_client_queue(void);
void send_clientID_to_client(void);


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

} ClientMessage; 

struct clientIDMessage
{
    long mType;
    int clientID;

} clientIDMessage;

