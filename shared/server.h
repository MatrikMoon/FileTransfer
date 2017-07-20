#ifndef _SERVER_H
#define _SERVER_H

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <signal.h>

#include "connection.h"

#define BUFLEN 4096

class Server : public Connection {
    //TCP
    int sockfd;

    //UDP
    int sock, port;
    unsigned int slength;
    struct sockaddr_in server;

    //Parser setup
    typedef int (*PARSER)(Server *s, char * buf, int length);
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
    int receiveTCP(PARSER p);
    int receive_low(PARSER p);
    static void *waitForRecvFunc(void * v);
    int connectTCP(std::string, std::string);
    void resetTCP();
    //UDP
    void sendUDP(std::string);
    void sendUDP(char * buf);
    void sendUDP(char * buf, int length);
    int receiveUDP(PARSER p);
    int receiveUDP_low(PARSER p);
    static void *waitForUDPFunc(void * v);
    int connectUDP(std::string, std::string);
};

#endif