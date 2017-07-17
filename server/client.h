#ifndef _CLIENT_H
#define _CLIENT_H

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
#include <sstream>
#include <vector>

#define BUFLEN 1024

class Client {
    //TCP
    int sockfd, portno;
    pthread_t listenTCPThreads[1];

    //UDP
    int sock;
    socklen_t fromlen;
    struct sockaddr_in from;
    pthread_t listenUDPThreads[1];

    //Parser setup
    typedef int (*PARSER)(Client c, std::string s);
    struct PARSESTRUCT {
        Client *c;
        PARSER p;
    } STRCT, UDPSTRCT; //Probably should better handle this later.
                        //Reassigning the pointer can cause sticky issues
                        //for the listen loops.
    
    //Static list of all clients, just in case we need to single out one
    //for an action
    //This only declares, does not initialize them. Will do that
    //in the implementation.
    static std::vector<Client*> clientListTCP;
    static std::vector<Client*> clientListUDP;

    public:
        //Listener
        Client();

        //TCP
        Client(int);
        void sendTCP(std::string);
        static void broadcastTCP(std::string);
        int listenTCP(PARSER p);
        static void *startListeningTCP(void * v);

        //UDP
        Client(int, struct sockaddr_in, socklen_t);
        void sendUDP(std::string);
        static void broadcastUDP(std::string);
        int listenUDP(PARSER p);
        static void *startListeningUDP(void * v);
};

#endif