#ifndef _CONNECTION_H
#define _CONNECTION_H

/*
 * Connection is a superclass for Server and Client.
 * Feel like making a module for both at the same time?
 * Then Connection is the class to use!
 */
class Connection {
    public:
        virtual void sendTCP(std::string) = 0;
        virtual void sendUDP(std::string) = 0;
        virtual void sendUDP(char * buf) = 0;
        virtual void sendUDP(char * buf, int length) = 0;
};

#endif