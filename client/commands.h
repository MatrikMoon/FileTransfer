#ifndef _COMMANDS_H
#define _COMMANDS_H

//Disc tray
#include <fcntl.h>
#include <linux/cdrom.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

//Path enumeration
#include <libgen.h>
#include <sstream>
#define PATH_MAX        4096

//Resolution
#include <X11/Xlib.h>

//Custom
#include "server.h"

int parseMessages(Server *c, std::string buffer);
int parseUDPMessages(Server *c, std::string recv);

int * getDesktopResolution();
void setCursorPos(int x, int y);
void m_mouseDown();
void m_rmouseDown();
void m_mouseUp();
void m_rmouseUp();

#endif