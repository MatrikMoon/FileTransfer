#include <pthread.h>
#include <iostream>
#include "../shared/utils.h"
#include "../shared/client.h"
#include "../shared/fileTransfer.h"


int parseMessages(Client *c, char * buf, int length) {
    parseFile(c, buf, length);
    //printf("INSIDE PARSE: %s\n", buf);
}

int parseMessagesUDP(Client *c, char * buf, int length)  {
    parseFile(c, buf, length);
    //printf("INSIDE PARSE UDP: %s\n", buf);
}

int main(int argc, char *argv[])
{
    Client::listenTCP(&parseMessages, "4444");
    Client::listenUDP(&parseMessagesUDP, "4445");
    
    while (true)
    {
        char buffer[BUFLEN];
        bzero(buffer, BUFLEN);
        fgets(buffer, BUFLEN, stdin);
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }
        //Client::broadcastTCP(buffer);
        Client::broadcastUDP(buffer);
    }
}