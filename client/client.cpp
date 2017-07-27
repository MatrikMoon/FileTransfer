#include "client.h"
#include "commands.h"

int main(int argc, char *argv[])
{
	//Prevent termination if our pipe breaks
	signal(SIGPIPE, SIG_IGN);
    if (argc < 3 && argc != 2) {
        printf("Usage: %s host port\n", argv[0]);
        exit(0);
    }
    else if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help")) == 0) {
        printf("Once you are connected to a server:\n");
        printf("/file-send [filename]  --  sends file to server\n");
        printf("/file-req [filename]   --  requests the server to send you a file\n");
        printf("/file-list             --  server lists files available for download\n");
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
        if (strncmp(buffer, "/file-send ", 11) == 0) {
            send_file(&server, &buffer[11]);
        }
        server.sendTCP(buffer);
    }
}