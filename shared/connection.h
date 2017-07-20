#ifndef _CONNECTION_H
#define _CONNECTION_H

class Connection {
    public:
        virtual void sendTCP(std::string) = 0;
        virtual void sendUDP(std::string) = 0;
        virtual void sendUDP(char * buf) = 0;
        virtual void sendUDP(char * buf, int length) = 0;
};

#endif