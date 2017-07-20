#ifndef _FILETRANSFER_H
#define _FILETRANSFER_H

#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/md5.h>

#include "server.h"

#define CHUNK_SIZE 16

struct FILEPARTS {
    int full_size;
    int data_size;
    char * data;
    char * full_header;
    std::string md5;
};

struct FILESTATS {
    int size;
    FILEPARTS * parts;
    int partsize;
    std::string md5;
};

int parseFile(Connection *c, char * buf, int length);
std::string md5_file(std::string path);
std::string md5(unsigned char * buffer);
std::string build_file_header(std::string file, FILESTATS &f);
int calculate_chunk_number(std::string file) ;
int get_file_size(std::string file);
unsigned char * get_chunk_data(std::string file, int chunk_number, int &data_size);
char * build_chunk_header(std::string file, std::string file_md5, int chunk_number, FILEPARTS &f);
int send_chunk_patch(Connection *c, std::string file, std::string super_md5, int chunk_number);
int send_file(Connection *c, std::string file);
std::string get_chunk_md5(std::string file, int chunk_number);
int append_to_file(std::string file, char * buf, int length);

#endif