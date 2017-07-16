#ifndef _SERVER_H
#define _SERVER_H

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <signal.h>

#define DEFAULT_BUFLEN 65535
#define BUFLEN 1024

class Server {
    //TCP
    int sockfd;

    //UDP
    int sock, port;
    unsigned int slength;
    struct sockaddr_in server;

    public:
    //Parser setup
    pthread_t receiveTCPThreads[1]; //FIXME: Does this do bad thingies
    typedef int (*PARSER)(Server c, std::string buffer);
    struct PARSESTRUCT {
        Server *s;
        PARSER p;
    } STRCT;

    //TCP
    std::string hostTCP;
    std::string portTCP;
    
    //UDP
    std::string hostUDP;
    std::string portUDP;
    
    //TCP
    int connectTCP(std::string, std::string);
    void sendTCP(std::string);
    static void *waitForRecvFunc(void * v);
    int receive_low(PARSER p);
    int receive(PARSER p);
    void resetTCP();
    void sendTCPIntro();
    //UDP
    int connectUDP(std::string, std::string);
    void sendUDP(std::string);
    void receiveUDPThread();
    static void *receiveUDPThreadHelper(void *context);
    int receiveUDP();
};

#endif