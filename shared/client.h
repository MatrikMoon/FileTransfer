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

#include "connection.h"

#define BUFLEN 4096

class Client : public Connection {
    //Global
    std::string UUID;

    //Misc
    bool hasTCPb;
    bool hasUDPb;

    //TCP
    static std::vector<pthread_t*> listenTCPThreads;
    static std::vector<pthread_t*> readTCPThreads;
    static void internalTCPParser(void*, std::string);

    //UDP
    int sock;
    socklen_t fromlen;
    static std::vector<pthread_t*> listenUDPThreads;

    //Parser setup
    typedef int (*PARSER)(Client *c, char * s, int l);
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
    static std::vector<Client*> clientList;
    //static std::vector<Client*> clientListUDP; //Todo: New data structure?

    public:
        //Misc
        std::string getUUID();
        void setUUID(std::string);
        void attachTCP(int);
        void attachUDP(int, struct sockaddr_in, socklen_t);
        bool hasTCP();
        bool hasUDP();

        //Both
        bool equals(Client c);

        //Listener/Destructor
        Client();
        ~Client();

        //TCP
        int sockfd; //Initialized when tcp client is added to list
                    //Also public for comparision purposes
        Client(int);
        void sendTCP(std::string);
        void sendTCP(char * buf);
        void sendTCP(char * buf, int length);
        static void broadcastTCP(std::string);
        static int listenTCP(PARSER p, std::string port);
        static void *startListeningTCP(void * v);
        static void *listen_tcp_low(void * v);

        //UDP
        struct sockaddr_in from; //Needs to be public for comparison purposes
        Client(int, struct sockaddr_in, socklen_t);
        void sendUDP(std::string);
        void sendUDP(char *);
        void sendUDP(char *, int);
        static void broadcastUDP(std::string);
        static int listenUDP(PARSER p, std::string port);
        static void *startListeningUDP(void * v);
};

#endif