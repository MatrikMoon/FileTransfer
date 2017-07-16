#include "client.h"

int main(int argc, char *argv[])
{
	//Prevent termination if our pipe breaks
	signal(SIGPIPE, SIG_IGN);
    if (argc < 3) {
        printf("Usage: a.out host port\n");
        exit(0);
    }
	//Connect to server
    server.connectTCP(argv[1], argv[2]);

	//Create thread to receive data
    int rc = pthread_create(&threads[0], NULL, waitForRecvfunc, NULL);
    if (rc) {
        std::cout<<"THREAD CREATION FAILED\n";
        exit(-1);
    }

	//Loop for input
    while (1) {
        char buffer[256];
        printf("\n");
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);
        server.sendTCP(buffer);
    }
}