#include <pthread.h>
#include <iostream>
#include "../shared/utils.h"
#include "../shared/client.h"
#include "../shared/fileTransfer.h"


int parseMessages(Client *c, char * buf, int length) {
    printf("INSIDE PARSE: %s\n", buf);
}

int parseMessagesUDP(Client *c, char * buf, int length)  {
    parseFile(c, buf, length);
    printf("INSIDE PARSE UDP: %d\n", length);
}

int main(int argc, char *argv[])
{
    Client::listenTCP(&parseMessages, "4444");
    Client::listenUDP(&parseMessagesUDP, "4445");
    
    while (true)
    {
        char buffer[BUFLEN];
        printf("\n");
        bzero(buffer, BUFLEN);
        fgets(buffer, BUFLEN, stdin);
        //Client::broadcastTCP(buffer);
        Client::broadcastUDP(buffer);
    }
}