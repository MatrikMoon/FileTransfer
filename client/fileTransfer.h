#ifndef _FILETRANSFER_H
#define _FILETRANSFER_H

#include <iostream>
#include <string.h>
#include <fstream>

#include "server.h"

struct FILEPARTS {
    int size;
    std::string md5;
};

struct FILESTATS {
    int size;
    FILEPARTS parts;
    int partsize;
    std::string md5;
};

int parseFile(Server *c, std::string recv);

#endif