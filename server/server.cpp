#include <pthread.h>
#include <iostream>
#include "utils.h"
#include "client.h"


int parseMessages(Client c, std::string buf) {
    printf("INSIDE PARSE: %s\n", buf.c_str());
}

int parseMessagesUDP(Client c, std::string buf) {
    printf("INSIDE PARSE UDP: %s", buf.c_str());
}

int main(int argc, char *argv[])
{
    Client c;
    c.listenTCP(&parseMessages);
    c.listenUDP(&parseMessagesUDP);
    
    while (true)
    {
        char buffer[BUFLEN];
        printf("\n");
        bzero(buffer, BUFLEN);
        fgets(buffer, BUFLEN, stdin);
        c.broadcastTCP(buffer);
    }
}