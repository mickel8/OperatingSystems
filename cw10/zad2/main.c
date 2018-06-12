#define _GNU_SOURCE
#define _OPEN_THREADS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <netinet/in.h>
#include <endian.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <sys/stat.h>

#include "utils.h"

struct nameBox
{
    int busy;
    struct sockaddr_in *addr1;
    struct sockaddr_un *addr2;
    char *name;
};

// ======== USER CONSTANTS ======== //
#define maxClients 20
// ================================ //


// ======== USER VARIABLES ======== //
static int port;
static char path[30];
static int networkSocket;
static int localSocket;
static pthread_t terminalThread;
static pthread_t networkThread;
static struct nameBox clientsNames[maxClients];
static int clientsfds[maxClients];
static int clientsNmb = 0;
static int currentClient = 0;
static int taskNmb = 0;
static char *path_shm;

pthread_mutex_t x_mutex = PTHREAD_MUTEX_INITIALIZER;
// ================================ //


// ======= FUNCTIONS ======== //
void *terminal(void *);
void *network(void *);
int create_network_socket(void);
int create_local_socket(void);
void start_listen(int);
int create_new_epoll(void);
void add_socket_to_epoll(int, int);
int accept_new_connection(struct epoll_event);
void receive_from_client_on();
void receive_from_client_ol();
void send_info(char *, int);
void close_socket(int);
void int_action(int);
void set_sigint_handler(void);
int register_name(char *, struct sockaddr_in *, struct sockaddr_un *);
void unregister_name(char *);
void set_path_in_shm(void);
// ========================== //


int main(int argc, char **argv)
{
    
    if(argc != 3)
    {
        printf("Bad arg number");
        exit(EXIT_FAILURE);
    }
    
    port = atoi(argv[1]);
    strcpy(path, argv[2]);  

    set_path_in_shm();

    for(int i = 0; i < maxClients; i++) clientsNames[i].busy = 0;
    for(int i = 0; i < maxClients; i++) clientsfds[i] = 0;

    int res;

    res = pthread_create(&terminalThread, NULL, &terminal, NULL);
    if(res != 0) perror("pthread_create");
    res = pthread_create(&networkThread, NULL, &network, NULL);
    if(res != 0) perror("pthread_create");

    res = pthread_join(terminalThread, NULL);
    if(res != 0) perror("pthread_join");
    res = pthread_join(networkThread, NULL);
    if(res != 0) perror("pthread_join");

    return 0;
}


void *terminal(void *args)
{
    ssize_t sentBytes;

    while(1)
    {
        char *task = NULL;
        size_t n = 0;

        getline(&task, &n, stdin);

        for(int i = 0; i < strlen(task); i++)
        {
            if(task[i] == '\n') task[i] = '\0';
        }
               
        char taskNmbStr[12];
        sprintf(taskNmbStr, "%d", ++taskNmb);
        
        char *mess = calloc(MAX_MESS_LEN, sizeof(char));
        sprintf(mess, "%s %s %s", CALC, task, taskNmbStr);

        printf("\e[97mPrzyjęto zadanie: %s\e[39m\n", mess); 

        if(clientsNmb < 0) printf("Brak klientów do wysłania rządania, usuwam rządanie\n");
        
        for( ; currentClient < maxClients; currentClient++)
        {
            if(clientsNames[currentClient].busy == 1)
            {
                if(clientsNames[currentClient].addr1 == NULL)
                {
                    int c = sizeof(struct sockaddr_un);
                    sentBytes = sendto(localSocket, mess, strlen(mess) + 1, 0, (struct sockaddr *)clientsNames[currentClient].addr2, (socklen_t)c);
                    if(sentBytes == -1) perror("terminal -> sendto");
                }
                else if(clientsNames[currentClient].addr2 == NULL)
                {
                    int c = sizeof(struct sockaddr_in);
                    sentBytes = sendto(networkSocket, mess, strlen(mess) + 1, 0, (struct sockaddr *)clientsNames[currentClient].addr1, (socklen_t)c);
                    if(sentBytes == -1) perror("terminal -> sendto");
                }

                if(currentClient == maxClients -1) currentClient = 0;
                else currentClient++;

                break;
            }
            
        }


    } 

    return NULL;
}

void *network(void *args)
{

    int epollfd;

    set_sigint_handler();

    networkSocket = create_network_socket();
    localSocket = create_local_socket();

    printf("\e[96mNetwork socket fd: %d\n", networkSocket);
    printf("Local socket fd: %d \e[39m\n", localSocket);

    epollfd = create_new_epoll();

    add_socket_to_epoll(epollfd, networkSocket);
    add_socket_to_epoll(epollfd, localSocket);

    struct epoll_event events[maxClients]; 
    
    while(1)
    {
        int mess; 
        mess = epoll_wait(epollfd, events, 20, -1);
        if(mess == -1) perror("network -> epoll_wait");

        for(int i = 0; i < mess; i++)
        {
            if(events[i].data.fd == networkSocket)
            {
                receive_from_client_on();
            }
            else if(events[i].data.fd == localSocket)
            {
                receive_from_client_ol();
            }
            else 
            {
                printf("Odebrano nieznaną wiadomość\n");
            }
        }

    }
    
    return NULL;
}

int create_network_socket()
{
    int res;
    int sockfd;
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1) perror("create_network_socket -> socket");
    else printf("Utworzono gniazdo sieciowe\n");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htobe16(port);
    addr.sin_addr.s_addr = INADDR_ANY; 

    res = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    if(res == -1) perror("create_network_socket -> bind");
    else printf("Przypisano nazwę do gniazda sieciowego\n");

    return sockfd;
   
}

int create_local_socket()
{
    int res;
    int sockfd;

    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(sockfd == -1) perror("create_local_socket -> socket");
    else printf("Utworzono gniazdo lokalne\n");

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);

    res = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    if(res == -1) perror("create_local_socket -> bind");
    else printf("Przypisano nazwę do gniazda lokalnego\n");

    return sockfd;
}

int create_new_epoll()
{
    int epollfd;

    epollfd = epoll_create1(0);
    if(epollfd == -1) perror("network -> epoll_create1");

    return epollfd;
}

void add_socket_to_epoll(int epollfd, int socketfd)
{
    int res;

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = socketfd;

    res = epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &event);
    if(res == -1) perror("network -> epoll_ctl");

    printf("Dodano socket: %d do epoll'a\n", socketfd);
}

void receive_from_client_on()
{
    ssize_t readBytes;
    char *mess = calloc(MAX_MESS_LEN, sizeof(char));
    
    struct sockaddr_in *addr = malloc(sizeof(struct sockaddr_in));
    int addrLen = sizeof(struct sockaddr_in);

    readBytes = recvfrom(networkSocket, mess, MAX_MESS_LEN, 0, addr, (socklen_t *)&addrLen);
    if(readBytes == -1) 
    {
        perror("receive_from_client -> recv");
        return;
    }
    else if (readBytes == 0)
    {
        printf("Klient zamknął połączenie\n");
        //unregister_name(clientfd);
        return;
    }
    else if (readBytes > 0)
    {
        int registerNameRes;

        if(mess[0] == NAME[0])
        {

            char *body = calloc(strlen(mess) + 1 - 2, sizeof(char));
            char *type = calloc(2, sizeof(char));

            sscanf(mess, "%s %s", type, body);

            registerNameRes = register_name(body, addr, NULL);
            if(registerNameRes == -2)
            {
                int c = sizeof(struct sockaddr_in);
                ssize_t sentBytes = sendto(networkSocket, MAXC, 2, 0, (struct sockaddr *)addr, (socklen_t)c);
                if(sentBytes != 2) perror("NAME -> sendto");
            }
            else if (registerNameRes == -1)
            {
                int c = sizeof(struct sockaddr_in);
                ssize_t sentBytes = sendto(networkSocket, BUSY, 2, 0, (struct sockaddr *)addr, (socklen_t)c);
                if(sentBytes != 2) perror("NAME -> sendto");
            }
            else 
            {
                int c = sizeof(struct sockaddr_in);
                ssize_t sentBytes = sendto(networkSocket, FREE, 2, 0, (struct sockaddr *)addr, (socklen_t)c);
                if(sentBytes != 2) perror("NAME -> sendto1  ");
                else printf("Wyslano potwierdzenie\n");
            }
            
            free(body);
            free(type);
        }
        else if(mess[0] == UNREG[0])
        {

            char *body = calloc(strlen(mess) + 1 - 2, sizeof(char));
            char *type = calloc(2, sizeof(char));

            sscanf(mess, "%s %s", type, body);

            printf("Klient \e[91m%s\e[39m zarządał wyrejestrowania\n", body);
            unregister_name(body);

            free(body);
            free(type);
        }
        else if(mess[0] == RES[0])
        {
            char *body = calloc(strlen(mess) + 1 - 2, sizeof(char));
            char *type = calloc(2, sizeof(char));
            char *taskNmb = calloc(14, sizeof(char));

            sscanf(mess, "%s %s %s", type, body, taskNmb);

            printf("Wynik: %s %s\n", body, taskNmb);
            
            free(body);
            free(type);
        }
        else
        {
            printf("Otrzymano nieznany typ wiadomości\n");
        }
    }
    
    
}

void receive_from_client_ol()
{
    ssize_t readBytes;
    char *mess = calloc(MAX_MESS_LEN, sizeof(char));
    
    struct sockaddr_un *addr = malloc(sizeof(struct sockaddr_un));
    int addrLen = sizeof(struct sockaddr_un);

    readBytes = recvfrom(localSocket, mess, MAX_MESS_LEN, 0, addr, (socklen_t *)&addrLen);
    if(readBytes == -1) 
    {
        perror("receive_from_client -> recv");
        return;
    }
    else if (readBytes == 0)
    {
        printf("Klient zamknął połączenie\n");
        //unregister_name(clientfd);
        return;
    }
    else if (readBytes > 0)
    {
        int registerNameRes;

        
        if(mess[0] == NAME[0])
        {

            char *body = calloc(strlen(mess) + 1 - 2, sizeof(char));
            char *type = calloc(2, sizeof(char));

            sscanf(mess, "%s %s", type, body);

            registerNameRes = register_name(body, NULL, addr);
            if(registerNameRes == -2)
            {
                int c = sizeof(struct sockaddr_un);
                ssize_t sentBytes = sendto(localSocket, MAXC, 2, 0, (struct sockaddr *)addr, (socklen_t)c);
                if(sentBytes != 2) perror("NAME -> sendto");
            }
            else if (registerNameRes == -1)
            {
                int c = sizeof(struct sockaddr_un);
                ssize_t sentBytes = sendto(localSocket, BUSY, 2, 0, (struct sockaddr *)addr, (socklen_t)c);
                if(sentBytes != 2) perror("NAME -> sendto");
            }
            else 
            {
                int c = sizeof(struct sockaddr_un);
                ssize_t sentBytes = sendto(localSocket, FREE, 2, 0, (struct sockaddr *)addr, (socklen_t)c);
                if(sentBytes != 2) perror("NAME -> sendto");
                else printf("Wyslano potwierdzenie\n");
            }
            
            free(body);
            free(type);
        }
        else if(mess[0] == UNREG[0])
        {

            char *body = calloc(strlen(mess) + 1 - 2, sizeof(char));
            char *type = calloc(2, sizeof(char));

            sscanf(mess, "%s %s", type, body);

            printf("Klient \e[91m%s\e[39m zarządał wyrejestrowania\n", body);
            unregister_name(body);

            free(body);
            free(type);
        }
        else if(mess[0] == RES[0])
        {
            char *body = calloc(strlen(mess) + 1 - 2, sizeof(char));
            char *type = calloc(2, sizeof(char));
            char *taskNmb = calloc(14, sizeof(char));

            sscanf(mess, "%s %s %s", type, body, taskNmb);

            printf("Wynik: %s %s\n", body, taskNmb);
            
            free(body);
            free(type);
        }
        else
        {
            printf("Otrzymano nieznany typ wiadomości\n");
        }
    }
    
    
}


void close_socket(int sockfd)
{
    int res;

    res = close(sockfd);
    if(res == -1) perror("close_socket -> close");
    else printf("\nZamknięto gniazdo");
}

void int_action(int signo)
{
    shmdt(path_shm);
    pthread_cancel(terminalThread);
    close_socket(networkSocket);
    close_socket(localSocket);
    unlink(path);
    exit(EXIT_SUCCESS);
}

void set_sigint_handler()
{
    int res;

    struct sigaction intAct;
    intAct.sa_handler = int_action;
    sigemptyset(&intAct.sa_mask);
    intAct.sa_flags = 0;

    res = sigaction(SIGINT, &intAct, NULL);
    if(res == -1) perror("set_sigint_handler -> sigaction");
}

int register_name(char *name, struct sockaddr_in *addr1, struct sockaddr_un *addr2)
{
    printf("Próba zarejestrowania nazwy klienta: %s\n", name);

    int freeIndex = -1;

    for(int i = 0; i < maxClients; i++)
    {
        if(clientsNames[i].busy == 1)
        {
            if(strcmp(clientsNames[i].name, name) == 0) {
                printf("Nazwa: %s jest już zajęta\n", name);
                return -1;
            }
        }
        else freeIndex = i;

    }

    if(freeIndex == -1)
    {
        printf("Przyjęto maksymalną ilosc połączeń\n");
        return -2;
    }
    else 
    {
        clientsNames[freeIndex].name = calloc(strlen(name) + 1, sizeof(char));
        strcpy(clientsNames[freeIndex].name, name);

        clientsNames[freeIndex].busy = 1;

        if(addr1 == NULL)
        {
            clientsNames[freeIndex].addr2 = malloc(sizeof(struct sockaddr_un));

            clientsNames[freeIndex].addr2 -> sun_family = addr2 -> sun_family;
            strcpy(clientsNames[freeIndex].addr2 -> sun_path, addr2 -> sun_path);
            clientsNames[freeIndex].addr1 = NULL;
        }
        else
        {
            clientsNames[freeIndex].addr1 = malloc(sizeof(struct sockaddr_in));

            clientsNames[freeIndex].addr1 -> sin_addr.s_addr = addr1 -> sin_addr.s_addr;
            clientsNames[freeIndex].addr1 -> sin_family = addr1 -> sin_family;
            clientsNames[freeIndex].addr1 -> sin_port = addr1 -> sin_port;

            clientsNames[freeIndex].addr2 = NULL;
        }

        printf("\e[92m** Zarejestrowano nowego klienta: %s **\e[39m\n", name);
        clientsNmb++;
        return 0; 
    }


}

void unregister_name(char *name)
{
    int found = 0;

    for(int i = 0; i < maxClients; i++)
    {
        if(clientsNames[i].busy == 1)
        {
            if(strcmp(clientsNames[i].name, name) == 0)
            {
                printf("\e[91m** Wyrejestrowano nazwę klienta %s **\e[39m\n", clientsNames[i].name);
                clientsNames[i].busy = 0;
                free(clientsNames[i].name);
                if(clientsNames[i].addr1 == NULL) free(clientsNames[i].addr2);
                else if(clientsNames[i].addr2 == NULL) free(clientsNames[i].addr1);
                clientsNmb--;
                found = 1;
                break;
            }
        }
    }
    if(found == 0) printf("Nie znaleziono klienta \e[91m%s\e[39m w bazie\n", name);
}

void set_path_in_shm()
{
    key_t key = ftok(keyPath, PROJ_ID);
    if(key == -1) perror("set_path_in_shm -> ftok");

    int res = shmget(key, shm_size, S_IRWXU | IPC_CREAT);
    if(res == -1) perror("set_path_in_shm -> shmget");
    int shmid = res;

    path_shm = shmat(shmid, NULL, 0);
    if(path_shm == (void *)-1) perror("set_path_in_shm -> shmat");
    else
    {
        char *path_shm_tmp = path_shm;
        strcpy(path_shm_tmp, path);
    }
}