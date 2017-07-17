#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <pthread.h>
#include <iostream>
#include <string.h>
#include "utils.h"
#include "client.h"

void *startListening(void *v);
void *startListeningTCP(void *v);


int parseMessages(Client c, std::string buf) {
    printf("INSIDE PARSE: %s", buf.c_str());
}

int main(int argc, char *argv[])
{
    Client c;
    c.listenTCP(&parseMessages);
    
    while (true)
    {
        char buffer[BUFLEN];
        printf("\n");
        bzero(buffer, BUFLEN);
        fgets(buffer, BUFLEN, stdin);
        /*
        if (c != NULL)
        {
            c->sendTCP(buffer);
        }
        */
    }
}