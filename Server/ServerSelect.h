
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>


typedef struct
{
    struct sockaddr_in Addr;
    socklen_t Len;
    in_port_t Port;
    fd_set    Rfdset;
    int       Maxfd;
    int       Socket;
} tServerItem;

typedef struct
{
    struct sockaddr_in Addr;
    socklen_t Len;
    in_port_t Port;
    int Socket;
    char Ntopbuf[INET_ADDRSTRLEN];
}tClientItem;

typedef struct 
{
    tServerItem Server;
    tClientItem Client;
}tSocketItem;


char* Revstr(char* pStr);
void serverInit(tServerItem* pServer);
void exitSys(const char *pMsg);
void acceptNewConnection(tServerItem* pServer, tClientItem* pClient);
void receiveData(int Fd, fd_set* pRfdset, tClientItem* pClient);

