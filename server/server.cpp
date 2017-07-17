/* Creates a datagram server.  The port 
   number is passed as an argument.  This
   server runs forever */

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <pthread.h>
#include <iostream>
#include <string.h>
#include "utils.h"
#include "client.h"

void *startListening(void *v);
void *startListeningTCP(void *v);
void parseMessages(char *buf);

pthread_t threads[1];
static Client *c;

int main(int argc, char *argv[])
{
    int rc = pthread_create(&threads[0], NULL, startListeningTCP, NULL);
    if (rc)
    {
        std::cout << "THREAD CREATION FAILED\n";
        exit(-1);
    }

    while (true)
    {
        char buffer[256];
        printf("\n");
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);
        if (c != NULL)
        {
            c->sendTCP(buffer);
        }
    }
}

void *startListening(void *v)
{
    int sock, length, n;
    socklen_t fromlen;
    struct sockaddr_in server;
    struct sockaddr_in from;
    char buf[1024];

    std::string port = "10152";

    //Open socket instance
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        error("Opening socket");
    }
    length = sizeof(server);
    bzero(&server, length);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(port.c_str()));

    //Bind to socket
    if (bind(sock, (struct sockaddr *)&server, length) < 0)
        error("binding");
    fromlen = sizeof(struct sockaddr_in);

    //Receive from various clients
    while (1)
    {
        //Recieve
        n = recvfrom(sock, buf, 1024, 0, (struct sockaddr *)&from, &fromlen);
        if (n < 0)
        {
            error("recvfrom");
        }

        //Set the static client to this one
        c = new Client(sock, from, fromlen);

        printf("\n%s\n", buf);
        bzero(&buf, 1024);
    }
    return 0;
}

void *startListeningTCP(void *v)
{
    std::cout<<"BEGINNING OF TCP\n";
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    std::string port = "10150";

    std::cout<<"BEGINNING OF SOCKET\n";
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(port.c_str());
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    std::cout<<"BEGINNING OF BIND\n";
    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    std::cout<<"BEGINNING OF ACCEPT\n";
    newsockfd = accept(sockfd,
                       (struct sockaddr *)&cli_addr,
                       &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");

    std::cout<<"BEGINNING OF ACCEPTED\n";

    bzero(buffer, 256);

    //Set the static client to this one
    c = new Client(newsockfd);

    //Receive from various clients
    while (TRUE)
    {
        //Recieve
        n = read(newsockfd, buffer, 255);
        if (n < 0)
        {
            error("ERROR reading from socket");
        }

        printf("\n%s\n", buffer);
        bzero(&buffer, 256);
    }
    close(newsockfd);
    close(sockfd);
    return 0;
}