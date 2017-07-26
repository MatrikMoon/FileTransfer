#include "client.h"
#include "commands.h"

int main(int argc, char *argv[])
{
	//Prevent termination if our pipe breaks
	signal(SIGPIPE, SIG_IGN);
    if (argc < 3) {
        printf("Usage: a.out host port\n");
        exit(0);
    }
	//Connect to server
    if (!server.connectTCP(argv[1], argv[2])) {
        return 1;
    }

    sendTCPIntro(&server);

    srand(time(0));
    
	//Create thread to receive data
    server.receiveTCP(&parseMessages);

    if (!server.connectUDP(argv[1], "4445")) {
        return 1;
    }

    server.receiveUDP(&parseUDPMessages);

	//Loop for input
    while (true) {
        char buffer[BUFLEN];
        bzero(buffer, BUFLEN);
        fgets(buffer, BUFLEN, stdin);
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }
        server.sendTCP(buffer);
    }
}