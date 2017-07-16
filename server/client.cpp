#include "client.h"
#include "utils.h"

Client::Client(int s, struct sockaddr_in a, socklen_t length)
{
    sock = s;
    from = a;
    fromlen = length;
}

Client::Client(int s)
{
    sockfd = s;
}

void Client::send(std::string buf)
{
    int n = sendto(sock, buf.c_str(), strlen(buf.c_str()), 0, (struct sockaddr *)&from, fromlen);
    if (n < 0)
    {
        error("sendto");
    }
}

void Client::sendTCP(std::string buf)
{
    buf += "<EOF>";
    int n = write(sockfd, buf.c_str(), strlen(buf.c_str()));
    if (n < 0)
        error("ERROR writing to socket");
}