#define serverNmb 11 // Number used to create server key
#define clientNmb 15 // Number used to create client key

#define CL_QUEUEID 1
#define maxClientsNmb 5


struct queueKeyMessage
{
    long mType;
    key_t qKey;
} queueKeyMessage; 