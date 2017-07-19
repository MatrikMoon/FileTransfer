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
#include <uuid/uuid.h>

#define BUFLEN 1024

class Client {
    //Global
    std::string UUID;

    //TCP
    int sockfd; //Initialized when tcp client is added to list
    static std::vector<pthread_t*> listenTCPThreads;
    static std::vector<pthread_t*> readTCPThreads;

    //UDP
    int sock;
    socklen_t fromlen;
    static std::vector<pthread_t*> listenUDPThreads;

    //Parser setup
    typedef int (*PARSER)(Client *c, std::string s);
    struct PARSESTRUCT {
        Client *c;
        PARSER p;
        std::string TCPPort;
        std::string UDPPort;
    };
    
    //Static list of all clients, just in case we need to single out one
    //for an action
    //This only declares, does not initialize them. Will do that
    //in the implementation.
    static std::vector<Client*> clientListTCP;
    static std::vector<Client*> clientListUDP; //Todo: New data structure?

    public:
        //Misc
        std::string getUUID();
        void fillUUID();

        //Both
        bool equals(Client c);

        //Listener/Destructor
        ~Client();

        //TCP
        Client(int);
        void sendTCP(std::string);
        static void broadcastTCP(std::string);
        static int listenTCP(PARSER p, std::string port);
        static void *startListeningTCP(void * v);
        static void *listen_tcp_low(void * v);

        //UDP
        struct sockaddr_in from; //Needs to be public for comparison purposes
        Client(int, struct sockaddr_in, socklen_t);
        void sendUDP(std::string);
        static void broadcastUDP(std::string);
        static int listenUDP(PARSER p, std::string port);
        static void *startListeningUDP(void * v);
};

#endif