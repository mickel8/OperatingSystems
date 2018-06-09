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

#include "utils.h"

// ======= FUNCTIONS ======= //
void create_and_connect_network_socket(void);
void create_and_connect_local_socket(void);
void send_name_to_server(void);
void close_socket(void);
void int_action(int);
void set_sigint_handler(void);
// ========================= //


// ======= USER VARIABLES ======= //
static int connectionVersion;
static int sockfd;

static char *name     = NULL;
static char *pathname = NULL;
static char *ipAdress = NULL;
static char *port     = NULL;
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

    while(1)
    {

    }


    close_socket();

    return 0;
}


void create_and_connect_network_socket()
{
    int res;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) perror("create_network_socket -> socket");
    else printf("Utworzono gniazdo sieciowe\n");
   
    // ===== Connect ===== //
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htobe16(atoi(port));
    res = inet_aton(ipAdress, &addr.sin_addr);
    if(res == 0) perror("networkConnection -> inet_aton");
    
    res = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    if(res == -1) perror("client -> connect");
    else printf("Połączono gniazdo sieciowe z serwerem\n");
}

void create_and_connect_local_socket()
{
    int res;

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd == -1) perror("create_local_socket -> socket");
    else printf("Utworzono gniazdo lokalne\n");
    
    // ===== Connect ===== //
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, pathname);

    res = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    if(res == -1) perror("client -> connect");
    else printf("Połączono gniazdo lokalne z zerwerem\n");

}

void send_name_to_server()
{
    ssize_t sentBytes;
    ssize_t readBytes;
    char *line = calloc(strlen(name) + 1, sizeof(char));
    char *mess_type = calloc(strlen(NAME)+ 1, sizeof(char));
    strcpy(line, name);

    // send info about name
    sentBytes = send(sockfd, NAME, sizeof(NAME), 0); 
    if(sentBytes != sizeof(NAME)) perror("send_name_to_server -> send");
    
    // send len of name
    long nameLen = htonl(strlen(line) + 1);
    sentBytes = send(sockfd, &nameLen, sizeof(nameLen), 0);
    if(sentBytes != sizeof(nameLen)) perror("send_name_to_server -> send");

    // send name
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
}

void close_socket()
{
    int res;

    res = shutdown(sockfd, SHUT_RDWR);
    if(res == -1) perror("close_socket -> shutdown");

    res = close(sockfd);
    if(res == -1) perror("close_socket -> close");
    else printf("Zamknięto gniazdo\n");
}

void int_action(int signo)
{
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