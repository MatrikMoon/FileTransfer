#include "client.h"
#include "utils.h"

//Static list of all clients, just in case we need to single out one
//for an action
std::vector<Client*> Client::clientList;

//Read thread vector
std::vector<pthread_t*> Client::readTCPThreads;
std::vector<pthread_t*> Client::listenTCPThreads;
std::vector<pthread_t*> Client::listenUDPThreads;

//Both
bool Client::equals(Client c) {
    return this->UUID == c.UUID;
}

std::string Client::getUUID() {
    return UUID;
}

void Client::setUUID(std::string id) {
    UUID = id;
}

void Client::attachTCP(int s) {
    sockfd = s;
    hasTCPb = true; //Set indicator to say we have tcp on this client
    printf("ATTACH TCP: %s\n", UUID.c_str());
}

void Client::attachUDP(int s, struct sockaddr_in a, socklen_t length) {
    sock = s;
    from = a;
    fromlen = length;
    hasUDPb = true; //Set indicator to say we have udp on this client
    printf("ATTACH UDP: %s\n", UUID.c_str());

    //Acknowledge the attachment by sending any data at all
    sendUDP(const_cast<char*>(">/ack"));
}

bool Client::hasTCP() {
    return hasTCPb;
}

bool Client::hasUDP() {
    return hasUDPb;
}

void Client::setHasTCP(bool b) {
    hasTCPb = b;
}

void Client::setHasUDP(bool b) {
    hasUDPb = b;
}

Client::~Client() {
    printf("Client with UUID: %s and sockfd: %d destroyed.\n", UUID.c_str(), sockfd);
}

//TCP
Client::Client(int s)
{
    sockfd = s;
    hasTCPb = true;
    hasUDPb = false; //No udp to be seen if we're in this constructor
}

void Client::broadcastTCP(std::string buf)
{
    for (int i = 0; i < clientList.size(); i++) {
        if (clientList.at(i)->hasTCP()) {
            clientList.at(i)->sendTCP(buf);
        }
    }
}

void Client::broadcastUDP(std::string buf)
{
    for (int i = 0; i < clientList.size(); i++) {
        if (clientList.at(i)->hasUDP()) {
            clientList.at(i)->sendUDP(buf);
        }
    }
}

void Client::sendTCP(std::string buf)
{
    sendTCP(const_cast<char*>(buf.c_str()));    
}

void Client::sendTCP(char * buf) {
    sendTCP(buf, strlen(buf));
}

void Client::sendTCP(char * buf, int length) {
    //Append <EOF> to the end.
    char sends[length + 5]; //+5 for <EOF>
    memcpy(sends, buf, length); //Memcpy just in case of null bytes
    strcpy(&sends[length], "<EOF>"); //Copy it over

    int n = write(sockfd, sends, length + 5);
    if (n < 0) {
        error("Error writing to socket.");
    }
}

int Client::listenTCP(PARSER p, std::string port) {
    //Create a new structure that isn't bounded by our current scope
    //Ideally this should be freed when we're done using it,
    //but because in an ideal situation we're never done,
    //I'll just leave it for now
    PARSESTRUCT *pst = static_cast<PARSESTRUCT*>(malloc(sizeof(PARSESTRUCT)));
    pst->p = p;
    pst->TCPPort = port;
    pthread_t *pth = static_cast<pthread_t*>(malloc(sizeof(pthread_t)));
    listenTCPThreads.push_back(pth);
    
    int rc = pthread_create(pth, NULL, &startListeningTCP, pst);
    if (rc)
    {
        std::cout << "THREAD CREATION FAILED\n";
    }
    return rc;
}

void *Client::startListeningTCP(void *v)
{
    PARSESTRUCT *p = static_cast<PARSESTRUCT*>(v);

    int newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
        return 0;
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    int portno = atoi(p->TCPPort.c_str());
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    //Keep accepting new clients
    while (true) {
        int newsockfd = accept(sockfd,
                        (struct sockaddr *)&cli_addr,
                        &clilen);
        if (newsockfd < 0) {
            error("ERROR on accept");
        }

        //Add our client to the list
        Client * c = new Client(newsockfd);
        //clientList.push_back(c); //Don't push until we get an ID

        //Start listener for our new client
        //Create a new structure that isn't bounded by our current scope
        //Ideally this should be freed when we're done using it,
        //but because in an ideal situation we're never done,
        //I'll just leave it for now
        PARSESTRUCT *pst = static_cast<PARSESTRUCT*>(malloc(sizeof(PARSESTRUCT)));
        pst->p = p->p;
        pst->c = c;

        pthread_t *pth = static_cast<pthread_t*>(malloc(sizeof(pthread_t)));
        readTCPThreads.push_back(pth);
        int rc = pthread_create(pth, NULL, &Client::listen_tcp_low, pst);
        if (rc)
        {
            std::cout << "THREAD CREATION FAILED\n";
        }
    }
    
    return 0;
}

//Helper function so we can avoid having a nasty
//repetitive few lines of code in listen_tcp_low()
void Client::internalTCPParser(void * v, std::string s) {
    PARSESTRUCT *p = static_cast<PARSESTRUCT*>(v);
    if (strncmp(s.c_str(), ">/connect ", 10) == 0) {
        //See if the client already exists as a UDP connection
        //If so, attach to it and clean up our mess
        std::string UUID = &(s.c_str()[10]);
        for (int i = 0; i < clientList.size(); i++) {
            if (!clientList.at(i)->hasTCP() && (clientList.at(i)->getUUID() == UUID)) {
                clientList.at(i)->attachTCP(p->c->sockfd);
                delete p->c;
                p->c = clientList.at(i);
                return;
            }
            else if (clientList.at(i)->hasTCP() && (clientList.at(i)->getUUID() == UUID)) {
                return; //This client is already connected, we have nothing to do here
            }
        }
        //If we make it here, this is a new client connecting
        clientList.push_back(p->c);
        p->c->setUUID(UUID); //Set our uuid
        return;
    }
    p->p(p->c, (char*)s.c_str(), s.length());   //parse everything up to <EOF>
}


//TODO: Rewrite this to accept null bytes from the socket, like UDP
void *Client::listen_tcp_low(void * v) {
    PARSESTRUCT *p = static_cast<PARSESTRUCT*>(v);

    //Receive from a single
    while (true)
    {
        char buffer[BUFLEN];
        bzero(buffer, BUFLEN);

        //Recieve
        int n = read(p->c->sockfd, buffer, BUFLEN);
        if (n <= 0)
        {
            printf("Error reading from socket.\n");
            break; //Break out and remove client from list
        }

        //p->p(p->c, buffer, n);

        //Break up the chunks of streams by <EOF>
        std::string temp_str(buffer, n); //Takes null bytes too
        while (temp_str.find("<EOF>") != -1)
        {   //While <EOF> is found in the buffer
            //std::cout << "LOOP: " << loopcount << " DATA: " << temp_str << "\n";

            if (temp_str.find("<EOF>") != (temp_str.length() - 5))
            {
                //printf("ALIGNING\n");
                std::string s = temp_str.substr(0, temp_str.find("<EOF>"));                                                       //if <EOF> isn't at the end of the buffer
                internalTCPParser(v, s);
                temp_str = temp_str.substr(temp_str.find("<EOF>") + 5);           //set the rest to the beginning of the next string
            }

            else
            {
                std::string s = temp_str.substr(0, temp_str.find("<EOF>"));
                internalTCPParser(v, s);
                temp_str = ""; //Set string back to empty
            }
        }
    }

    close(p->c->sockfd); //Close socket

    //We've closed the socket! Remove from us from the list.
    for (int i = 0; i < clientList.size(); i++) {
        //If we have a UUID
        if (p->c->getUUID() != "") {
            if (clientList.at(i)->getUUID() == p->c->getUUID()) {
                //Remove the client from the list
                //Notice we don't check for an existing UDP
                //connection. This is because there's no
                //way to tell if a UDP connection is alive,
                //and if there was one, it's probably dead
                //too if the TCP one is.
                delete clientList.at(i); //Delete object
                clientList.erase(clientList.begin() + i); //Remove element
                free(p); //Free malloc'd struct
                break;
            }
        }
        //No UUID? Good news! We're not in the list yet
        //If we don't have a UUID we can't have a UDP
        //counterpart anyway, so no worries.
        else {
            delete p->c; //Delete object
            free(p); //Free malloc'd struct
            break;
        }
    }
}

//UDP
Client::Client(int s, struct sockaddr_in a, socklen_t length)
{
    sock = s;
    from = a;
    fromlen = length;
    hasUDPb = true;
    hasTCPb = false; //If we're in this constructor, we don't have a tcp connection yet
}

void Client::sendUDP(std::string buf)
{
    sendUDP((char*)buf.c_str());
}

void Client::sendUDP(char * buf)
{
    sendUDP(buf, strlen(buf));
}

void Client::sendUDP(char * buf, int length)
{
    int n = sendto(sock, buf, length, 0, (struct sockaddr *)&from, fromlen);
    if (n < 0)
    {
        error("sendto");
    }
}

int Client::listenUDP(PARSER p, std::string port) {
    //Create a new structure that isn't bounded by our current scope
    //Ideally this should be freed when we're done using it,
    //but because in an ideal situation we're never done,
    //I'll just leave it for now
    PARSESTRUCT *pst = static_cast<PARSESTRUCT*>(malloc(sizeof(PARSESTRUCT)));
    pst->p = p;
    pst->UDPPort = port;
    pthread_t *pth = static_cast<pthread_t*>(malloc(sizeof(pthread_t)));
    listenUDPThreads.push_back(pth);
    
    int rc = pthread_create(pth, NULL, &startListeningUDP, pst);
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

    //Open socket instance
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        error("Opening socket");
    }
    length = sizeof(server);
    bzero(&server, length);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(p->UDPPort.c_str()));

    //Bind to socket
    if (bind(sock, (struct sockaddr *)&server, length) < 0)
        error("binding");
    socklen_t fromlen = sizeof(struct sockaddr_in);

    //Receive from various clients
    while (true)
    {
        //Recieve
        struct sockaddr_in from;
        n = recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *)&from, &fromlen);
        if (n <= 0)
        {
            //TODO: Remove client from list... maybe?
            error("recvfrom");
        }

        //Pointer to the client
        Client *c;

        bool connection_message = false;
        std::string UUID;
        //If this is a connection message...
        if (strncmp(buf, ">/connect ", 10) == 0) {
            connection_message = true;
            UUID = &buf[10];
        }

        //Add the udp client to the list if it's not already there
        bool exists = false;
        for (int i = 0; i < clientList.size(); i++) {
            //If the currently tested client has udp
            if (clientList.at(i)->hasUDP()) {
                //Compare addresses of existing clients and the new one
                if ((clientList.at(i)->from.sin_addr.s_addr == from.sin_addr.s_addr) && 
                    (clientList.at(i)->from.sin_port == from.sin_port)) {
                    //Ah, so we exist. Delete the newly created object
                    //and replace it with the existing one.
                    exists = true;
                    c = clientList.at(i);
                }
            }
            //If this is a connection message, let's check to see
            //if the connecting client already has a TCP counterpart
            else if (connection_message) {
                if (clientList.at(i)->getUUID() == UUID) {
                    exists = true;
                    c = clientList.at(i);
                    c->attachUDP(sock, from, fromlen);
                }
            }
        }
        //If it's none of the above, it's a new connection with a UUID
        //Add it to the list. If it's not this either, it's SOL
        if (!exists && connection_message) {
            c = new Client(sock, from, fromlen);
            clientList.push_back(c);
            c->setUUID(UUID);
        }

        if (c) {
            p->p(c, buf, n);
        }

        bzero(&buf, BUFLEN);
        bzero(&from, sizeof(sockaddr_in));
    }
    return 0;
}