#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <signal.h>
#include <netinet/in.h>
#include <endian.h>
#include <arpa/inet.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#include <sys/stat.h>

#include "utils.h"

// ======= FUNCTIONS ======= //
void create_and_connect_network_socket(void);
void create_and_connect_local_socket(void);
void send_name_to_server(void);
void close_socket(void);
void int_action(int);
void set_sigint_handler(void);
void get_server_name(void);
// ========================= //


// ======= USER VARIABLES ======= //
static int connectionVersion;
static int sockfd;

static char *name     = NULL;
static char *pathname = NULL;
static char *ipAdress = NULL;
static char *port     = NULL;

static char *serverName;
// ============================== //


int main(int argc, char **argv)
{

    if(argc != 4) 
    {
        printf("Bad number of args\n");
        exit(EXIT_FAILURE);
    }

    name = calloc(strlen(argv[1]) + 1, sizeof(char));
    strcpy(name, argv[1]);
    
    set_sigint_handler();
    get_server_name();

    connectionVersion = atoi(argv[2]);

    if(connectionVersion == 0)
    {
        ipAdress = strtok(argv[3], ":");
        port = strtok(NULL, ":");
        pathname = "NONE";

        create_and_connect_network_socket();
    }
    else if(connectionVersion == 1)
    {
        pathname = argv[3];
        ipAdress = "NONE";
        port = "NONE";

        create_and_connect_local_socket();
    }

    send_name_to_server();


    ssize_t readBytes;
    ssize_t sentBytes;
        
    char *mess = calloc(MAX_MESS_LEN, sizeof(char));
    
    while(1)
    {
        
        readBytes = recv(sockfd, mess, MAX_MESS_LEN, 0);
        if(readBytes == -1) 
        {
            perror("main -> recv");
            break;
        }
        else if (readBytes == 0)
        {
            printf("Server zamknął połączenie\n");
            break;
        }
        else
        {
            if(mess[0] == CALC[0]) 
            {
        
                char *type = calloc(2, sizeof(char));

                printf("\e[96m** Odebrano wiadomość: %s **\e[39m\n", mess);

                int a; 
                char op; 
                int b; 
                int taskNmb;
                
                sscanf(mess, "%s %d %c %d %d", type, &a, &op, &b, &taskNmb);              	

                int result;
                if(op == '+') result = a + b;
                else if(op == '-') result = a - b;
                else if(op == '*') result = a * b;
                else if(op == '/') result = a / b;

                // prepare result to send
                char *resultStr = calloc(MAX_MESS_LEN, sizeof(char));
                sprintf(resultStr, "%s %d %d", RES, result, taskNmb);

                int resultLen = strlen(resultStr) + 1;
                // send result
                sentBytes = send(sockfd, resultStr, resultLen, 0);
                if(sentBytes != resultLen) perror("send_name_to_server -> send");
                

                printf("\e[92m** Wysłano odpowiedź %s **\e[39m\n", resultStr);

                free(type);
            }
            else
            {
                printf("Nieznany rodzaj wiadomości\n");       
            }
        }
    
    }


    close_socket();
    free(name);

    return 0;
}


void create_and_connect_network_socket()
{
    int res;
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1) perror("create_network_socket -> socket");
    else printf("Utworzono gniazdo sieciowe\n");

    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htobe16(atoi(port));
    //addr.sin_addr.s_addr = INADDR_ANY;
    
    // // ===== Bind ===== //
    // res = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    // if(res == -1) perror("create_network_socket -> bind");
    // else printf("Przypisano nazwę do gniazda sieciowego\n");
   
    // ===== Connect ===== //
    res = inet_aton(ipAdress, &addr.sin_addr);
    if(res == 0) perror("networkConnection -> inet_aton");
    res = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    if(res == -1) perror("client -> connect");
    else printf("Połączono gniazdo sieciowe z serwerem\n");
}

void create_and_connect_local_socket()
{
    int res;

    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(sockfd == -1) perror("create_and_connect_local_socket -> socket");
    else printf("Utworzono gniazdo lokalne\n");


    // ===== Bind ====== //
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, pathname);

    res = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    if(res == -1) perror("create_and_connect_local_socket -> bind");
    else printf("Przypisano nazwę do gniazda lokalnego\n");
    
    strcpy(addr.sun_path, serverName);
    // ===== Connect ===== //
    res = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    if(res == -1) perror("create_and_connect_local_socket -> connect");
    else printf("Połączono gniazdo lokalne z zerwerem\n");

}

void send_name_to_server()
{
    ssize_t sentBytes;
    ssize_t readBytes;
    char *line = calloc(strlen(name) + 3, sizeof(char));
    char *mess_type = calloc(strlen(NAME)+ 1, sizeof(char));
    
    // send name
    sprintf(line, "%s %s", NAME, name);
    long nameLen = htonl(strlen(line) + 1);

    sentBytes = send(sockfd, line, ntohl(nameLen), 0);
    if(sentBytes != ntohl(nameLen)) perror("send_name_to_server -> send");

    // recv respond for name request
    readBytes = recv(sockfd, mess_type, 2, 0);
    if(readBytes == -1) perror("NAME -> recv");
    else if(readBytes == 0) printf("NAME -> klient zamknął połączenie");

    if(strcmp(mess_type, BUSY) == 0)
    {
        printf("Podana nazwa jest już zajęta\n");
        free(line);
        free(mess_type);
        close_socket();
        exit(EXIT_SUCCESS);
    }
    else if(strcmp(mess_type, MAXC) == 0)
    {
        free(line);
        free(mess_type);
        printf("Serwer obsługuje maksymalną ilość klientów\n");
        exit(EXIT_SUCCESS);
    }
    else if(strcmp(mess_type, FREE) == 0)
    {
        free(line);
        free(mess_type);
        printf("Pomyślnie zarejestrowano nazwę\n");
        return;
    }
    else 
    {
        printf("Nieznany rodzaj wiadomości: %s\n", mess_type);
    }
}

void unregister()
{
    ssize_t sentBytes;
    char *mess = calloc(strlen(name) + 3, sizeof(char));
    sprintf(mess, "%s %s", UNREG, name);

    sentBytes = send(sockfd, mess, strlen(mess) + 1, 0);
    if(sentBytes != strlen(mess) + 1) perror("unregister -> send");

    printf("Wyrejestrowano z serwera\n");

}

void close_socket()
{
    int res;

    res = close(sockfd);
    if(res == -1) perror("close_socket -> close");
    else printf("Zamknięto gniazdo\n");
}

void int_action(int signo)
{
    shmdt(serverName);
    unregister();
    close_socket();
    if(connectionVersion == 1) unlink(pathname);
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

void get_server_name()
{
    key_t key = ftok(keyPath, PROJ_ID);

    int shmid = shmget(key, shm_size, S_IRUSR);

    serverName = shmat(shmid, NULL, 0);
}