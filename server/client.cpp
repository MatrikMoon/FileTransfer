#include "client.h"
#include "utils.h"

//Static list of all clients, just in case we need to single out one
//for an action
std::vector<Client*> Client::clientListTCP;
std::vector<Client*> Client::clientListUDP;

//Listener
Client::Client() {
    //Just initizlize the class.
}

//TCP
Client::Client(int s)
{
    sockfd = s;
}

void Client::broadcastTCP(std::string buf)
{
    for (int i = 0; i < clientListTCP.size(); i++) {
        clientListTCP.at(i)->sendTCP(buf);
    }
}

void Client::broadcastUDP(std::string buf)
{
    for (int i = 0; i < clientListTCP.size(); i++) {
        clientListUDP.at(i)->sendUDP(buf);
    }
}

void Client::sendTCP(std::string buf)
{
    buf += "<EOF>";
    int n = write(sockfd, buf.c_str(), strlen(buf.c_str()));
    if (n < 0)
        error("ERROR writing to socket");
}

int Client::listenTCP(PARSER p) {
    STRCT.p = p;
    STRCT.c = this;
    int rc = pthread_create(&listenTCPThreads[0], NULL, &startListeningTCP, &STRCT);
    if (rc)
    {
        std::cout << "THREAD CREATION FAILED\n";
    }
    return rc;
}

void *Client::startListeningTCP(void *v)
{
    PARSESTRUCT *p = static_cast<PARSESTRUCT*>(v);

    std::cout<<"BEGINNING OF TCP\n";
    int newsockfd;
    socklen_t clilen;
    char buffer[BUFLEN];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    std::string port = "4444";

    std::cout<<"BEGINNING OF SOCKET\n";
    p->c->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (p->c->sockfd < 0) {
        error("ERROR opening socket");
        return 0;
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    p->c->portno = atoi(port.c_str());
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(p->c->portno);

    std::cout<<"BEGINNING OF BIND\n";
    if (bind(p->c->sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(p->c->sockfd, 5);
    clilen = sizeof(cli_addr);

    std::cout<<"BEGINNING OF ACCEPT\n";
    newsockfd = accept(p->c->sockfd,
                       (struct sockaddr *)&cli_addr,
                       &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");

    std::cout<<"BEGINNING OF ACCEPTED\n";

    bzero(buffer, BUFLEN);

    //Add our client to the list
    clientListTCP.push_back(new Client(newsockfd));

    //Receive from various clients
    while (true)
    {
        //Recieve
        n = read(newsockfd, buffer, BUFLEN);
        if (n < 0)
        {
            error("ERROR reading from socket");
        }

        p->p(*(p->c), buffer);

        bzero(&buffer, BUFLEN);
    }
    close(newsockfd);
    close(p->c->sockfd);
    return 0;
}

//UDP
Client::Client(int s, struct sockaddr_in a, socklen_t length)
{
    sock = s;
    from = a;
    fromlen = length;
}

void Client::sendUDP(std::string buf)
{
    int n = sendto(sock, buf.c_str(), strlen(buf.c_str()), 0, (struct sockaddr *)&from, fromlen);
    if (n < 0)
    {
        error("sendto");
    }
}

int Client::listenUDP(PARSER p) {
    UDPSTRCT.p = p;
    UDPSTRCT.c = this;
    int rc = pthread_create(&listenUDPThreads[0], NULL, &startListeningUDP, &UDPSTRCT);
    if (rc)
    {
        std::cout << "THREAD CREATION FAILED\n";
    }
    return rc;
}

void *Client::startListeningUDP(void *v)
{
    PARSESTRUCT *p = static_cast<PARSESTRUCT*>(v);
    int length, n;
    struct sockaddr_in server;
    char buf[BUFLEN];

    std::string port = "4445";

    //Open socket instance
    p->c->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (p->c->sock < 0)
    {
        error("Opening socket");
    }
    length = sizeof(server);
    bzero(&server, length);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(port.c_str()));

    //Bind to socket
    if (bind(p->c->sock, (struct sockaddr *)&server, length) < 0)
        error("binding");
    p->c->fromlen = sizeof(struct sockaddr_in);

    //Receive from various clients
    while (true)
    {
        //Recieve
        n = recvfrom(p->c->sock, buf, BUFLEN, 0, (struct sockaddr *)&(p->c->from), &(p->c->fromlen));
        if (n < 0)
        {
            error("recvfrom");
        }

        //Set the static client to this one
        //c = new Client(sock, from, fromlen);

        printf("\nPARSINGUDP: %s\n", buf);
        p->p(*(p->c), buf);

        bzero(&buf, BUFLEN);
    }
    return 0;
}