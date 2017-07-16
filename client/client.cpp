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
    if (server.connectTCP(argv[1], argv[2]) == 1) {
        return 1;
    }

	//Create thread to receive data
    server.receive(&parseMessages);

	//Loop for input
    while (1) {
        char buffer[256];
        printf("\n");
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);
        server.sendTCP(buffer);
    }
}