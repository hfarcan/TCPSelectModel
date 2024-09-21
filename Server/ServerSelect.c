#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "serverselect.h"

#define BUFFERSIZE      1024


int main(int argc, char* argv[])
{
    tSocketItem tcp;
    fd_set tempfdset;

    if (argc != 2)
    {
        fprintf(stderr, "wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    tcp.Server.Port = (in_port_t)strtoul(argv[1], NULL, 10);

    FD_ZERO(&tcp.Server.Rfdset);

    if ((tcp.Server.Socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        exitSys("Socket");

    serverInit(&tcp.Server);

    if (bind(tcp.Server.Socket, (struct sockaddr *)&tcp.Server.Addr, tcp.Server.Len) == -1)
        exitSys("Bind");

    FD_SET(tcp.Server.Socket, &tcp.Server.Rfdset);

    tcp.Server.Maxfd = tcp.Server.Socket;

    if (listen(tcp.Server.Socket, 8) == -1)
        exitSys("Listen");

    printf("Waiting for connections...\n");

    for (;;)
    {
        tempfdset = tcp.Server.Rfdset;

        if (select(tcp.Server.Maxfd + 1, &tempfdset, NULL, NULL, NULL) == -1)
            exitSys("Select");

        for (int i = 0; i <= tcp.Server.Maxfd; ++i)
        {
            if (FD_ISSET(i, &tempfdset))
            {
                if (i == tcp.Server.Socket)
                {
                    acceptNewConnection(&tcp.Server, &tcp.Client);
                }
                else
                {
                    receiveData(i, &tcp.Server.Rfdset,&tcp.Client);
                }
            }
        }
    }

    close(tcp.Server.Socket);
    return 0;
}

void acceptNewConnection(tServerItem* pServer, tClientItem* pClient)
{   
    pClient->Len = sizeof(pClient->Addr);
    if ((pClient->Socket = accept(pServer->Socket, (struct sockaddr *)&pClient->Addr, &pClient->Len)) == -1)
        exitSys("Accept Problem");

    inet_ntop(AF_INET, &pClient->Addr.sin_addr, pClient->Ntopbuf, INET_ADDRSTRLEN);
    printf("Connected: %s: %u\n", pClient->Ntopbuf, (unsigned)ntohs(pClient->Addr.sin_port));

    FD_SET(pClient->Socket, &(pServer->Rfdset));
    if (pClient->Socket > pServer->Maxfd)
        pServer->Maxfd = pClient->Socket;
}

void receiveData(int Fd, fd_set* pRfdset, tClientItem* pClient)
{
    char buf[BUFFERSIZE + 1];
    ssize_t result;

    if ((result = recv(Fd, buf, BUFFERSIZE, 0)) == -1)
        exitSys("Recv");

    if (result > 0)
    {
        buf[result] = '\0';
        printf("%ld bytes received from %s (%u): %s\n", (long)result, pClient->Ntopbuf, (unsigned)ntohs(pClient->Addr.sin_port), buf);
        Revstr(buf);
        
        if (send(Fd, buf, strlen(buf), 0) == -1)
            exitSys("Send Problem");
    }
    else
    {
        shutdown(Fd, SHUT_RDWR);
        close(Fd);
        FD_CLR(Fd, pRfdset);
    }
}

char* Revstr(char* pStr)
{
     size_t i,k;
    char temp; 

    for(i = 0; pStr[i] != '\0'; ++i)
        ;

    for(--i, k = 0; k < 1; ++k, --i)
    {
        temp = pStr[k];
        pStr[k] = pStr[i];
        pStr[i] = temp;
    }
    return pStr;
}

void serverInit(tServerItem* pServer)
{
    pServer->Addr.sin_family = AF_INET;
    pServer->Addr.sin_port = htons(pServer->Port);
    pServer->Addr.sin_addr.s_addr = htonl(INADDR_ANY);
    pServer->Len = sizeof(pServer->Addr);
}

void exitSys(const char *pMsg)
{
    perror(pMsg);
    exit(EXIT_FAILURE);
}
