#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

typedef struct {
    int Sock;
    struct addrinfo *Addrinfo;
}tConnection;

void exit_sys(const char *pMsg);
tConnection createConnection(const char *pHostname, const char *pPort);
void connectServer(tConnection *pConn);
void communicateServer(tConnection *pConn, const char *pHostname, const char *pPort);
void cleanup(tConnection *conn);

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "wrong number of arguments!..\n");
        exit(EXIT_FAILURE);
    }

    tConnection conn = createConnection(argv[1], argv[2]);
    connectServer(&conn);
    communicateServer(&conn, argv[1], argv[2]);
    cleanup(&conn);

    return 0;
}


tConnection createConnection(const char *pHostname, const char *pPort)
{
    tConnection conn;
    struct addrinfo hints = {0};
    int sresult;

    // Socket oluşturma
    conn.Sock = socket(AF_INET, SOCK_STREAM, 0);
    if (conn.Sock == -1)
        exit_sys("socket");

    // Adres bilgilerini alma
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((sresult = getaddrinfo(pHostname, pPort, &hints, &conn.Addrinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(sresult));
        exit(EXIT_FAILURE);
    }

    return conn;
}

void connectServer(tConnection *pConn)
{
    struct addrinfo *ri;

    for (ri = pConn->Addrinfo; ri != NULL; ri = ri->ai_next) {
        if (connect(pConn->Sock, ri->ai_addr, ri->ai_addrlen) != -1)
            break;
    }

    if (ri == NULL)
        exit_sys("connect");

    printf("Connected...\n");
    freeaddrinfo(pConn->Addrinfo);
}

void communicateServer(tConnection *pConn, const char *pHostname, const char *pPort)
{
    char buf[BUFFER_SIZE];
    char *pstr;
    ssize_t result;

    for (;;) {
        printf("Yazı giriniz: ");
        fgets(buf, BUFFER_SIZE, stdin);
        if ((pstr = strchr(buf, '\n')) != NULL)
            *pstr = '\0';

        if (!strcmp(buf, "quit"))
            break;

        if (send(pConn->Sock, buf, strlen(buf), 0) == -1)
            exit_sys("send");

        if ((result = recv(pConn->Sock, buf, BUFFER_SIZE, 0)) == -1)
            exit_sys("recv");

        if (result == 0)
            break;

        buf[result] = '\0';
        printf("%ld bytes received from %s (%s): %s\n", (long)result, pHostname, pPort, buf);
    }
}

void cleanup(tConnection *pConn)
{
    shutdown(pConn->Sock, SHUT_RDWR);
    close(pConn->Sock);
}

void exit_sys(const char *pMsg)
{
    perror(pMsg);
    exit(EXIT_FAILURE);
}
