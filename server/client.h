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

class Client {
    //TCP
    int sockfd;

    //UDP
    int sock;
    socklen_t fromlen;
    struct sockaddr_in from;

    public:
        Client(int);
        Client(int, struct sockaddr_in, socklen_t);
        void send(std::string);
        void sendTCP(std::string);
};

#endif