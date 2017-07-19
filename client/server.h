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

    //Parser setup
    typedef int (*PARSER)(Server s, std::string buffer);
    pthread_t receiveTCPThreads[1]; //FIXME: Does this do bad thingies
    pthread_t receiveUDPThreads[1]; //FIXME: Same ^
    struct PARSESTRUCT {
        Server *s;
        PARSER p;
    };


    public:
    //TCP
    std::string hostTCP;
    std::string portTCP;
    
    //UDP
    std::string hostUDP;
    std::string portUDP;
    
    //TCP
    void sendTCP(std::string);
    int receive(PARSER p);
    int receive_low(PARSER p);
    static void *waitForRecvFunc(void * v);
    int connectTCP(std::string, std::string);
    void resetTCP();
    void sendTCPIntro();
    //UDP
    void sendUDP(std::string);
    int receiveUDP(PARSER p);
    int receiveUDP_low(PARSER p);
    static void *waitForUDPFunc(void * v);
    int connectUDP(std::string, std::string);
};

#endif