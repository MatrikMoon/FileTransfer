#include "client.h"
#include "utils.h"

//Static list of all clients, just in case we need to single out one
//for an action
std::vector<Client*> Client::clientListTCP;
std::vector<Client*> Client::clientListUDP;

//Read thread vector
std::vector<pthread_t*> Client::readTCPThreads;
std::vector<pthread_t*> Client::listenTCPThreads;
std::vector<pthread_t*> Client::listenUDPThreads;

//Both
bool Client::equals(Client c) {
    //If this compares two uninitialized clients,
    //it'll return true even if they're not technically equal
    //It is important to initalize.
    /*
    if (this->sockfd != null && c.sockfd != null) {
        return this->sockfd == c.sockfd;
    }
    */
    //else if (this->)
    return this->UUID == c.UUID;
}

void Client::fillUUID() {
    uuid_t id;
    uuid_generate(id);
    char * uuid_ptr = new char[37];
    uuid_unparse(id, uuid_ptr);
    UUID = uuid_ptr;
    printf("Client with UUID: %s created.\n", UUID.c_str());
}

std::string Client::getUUID() {
    return UUID;
}

Client::~Client() {
    printf("Client with UUID: %s and sockfd: %d destroyed.\n", UUID.c_str(), sockfd);
}

//TCP
Client::Client(int s)
{
    sockfd = s;
    printf("SOCKFD: %d\n", s);
    fillUUID();
}

void Client::broadcastTCP(std::string buf)
{
    for (int i = 0; i < clientListTCP.size(); i++) {
        clientListTCP.at(i)->sendTCP(buf);
    }
}

void Client::broadcastUDP(std::string buf)
{
    for (int i = 0; i < clientListUDP.size(); i++) {
        clientListUDP.at(i)->sendUDP(buf);
    }
}

void Client::sendTCP(std::string buf)
{
    buf += "<EOF>";
    int n = write(sockfd, buf.c_str(), strlen(buf.c_str()));
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
        clientListTCP.push_back(c);

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

        //Break up the chunks of streams by <EOF>
        std::string temp_str = buffer;
        while (temp_str.find("<EOF>") != -1)
        {   //While <EOF> is found in the buffer
            //std::cout << "LOOP: " << loopcount << " DATA: " << temp_str << "\n";

            if (temp_str.find("<EOF>") != (temp_str.length() - 5))
            {                                                                     //if <EOF> isn't at the end of the buffer
                p->p(p->c, temp_str.substr(0, temp_str.find("<EOF>")));             //parse everything up to <EOF>
                temp_str = temp_str.substr(temp_str.find("<EOF>") + 5);           //set the rest to the beginning of the next string
            }

            else
            {
                p->p(p->c, temp_str.substr(0, temp_str.find("<EOF>")));
                temp_str = ""; //Set string back to empty
            }
        }
    }

    close(p->c->sockfd); //Close socket

    //We've closed the socket! Remove from us from the list.
    for (int i = 0; i < clientListTCP.size(); i++) {
        if (clientListTCP.at(i)->getUUID() == p->c->getUUID()) {
            delete clientListTCP.at(i); //Delete object
            clientListTCP.erase(clientListTCP.begin() + i); //Remove element
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
    fillUUID();
}

void Client::sendUDP(std::string buf)
{
    int n = sendto(sock, buf.c_str(), strlen(buf.c_str()), 0, (struct sockaddr *)&from, fromlen);
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
        Client *c = new Client(sock, from, fromlen);

        //Add the udp client to the list if it's not already there
        bool exists = false;
        for (int i = 0; i < clientListTCP.size(); i++) {
            if (clientListUDP.at(i)->equals(*c)) {
                //Ah, so we exist. Delete the newly created object
                //and replace it with the existing one.
                exists = true;
                delete c;
                c = clientListUDP.at(i);
            }
        }
        if (!exists) {
            clientListUDP.push_back(c);
        }

        p->p(c, buf);

        bzero(&buf, BUFLEN);
        bzero(&from, sizeof(sockaddr_in));
    }
    return 0;
}