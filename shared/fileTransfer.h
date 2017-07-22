#ifndef _FILETRANSFER_H
#define _FILETRANSFER_H

#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/md5.h>
#include <tr1/unordered_map>

#include "server.h"

//Data header chunk size
#define CHUNK_SIZE 16

//Forward declaration of filestats so we can use it in fileparts
struct FILESTATS;

//Holds stats about an individual data chunk
struct FILEPARTS {
    int chunk_id;
    int full_size;
    int data_size;
    char * data;
    char * full_header;
    std::string md5;
    std::string super_md5;
    FILESTATS * stats;

    ~FILEPARTS() {
        //delete[] data; //deleted during execution
        //delete[] full_header;
        //printf("FILEPARTS DELETED: %s\n", md5.c_str());
    }
};

//Holds stats about the current file being downloaded
struct FILESTATS {
    int size;
    int chunk_size;
    int parts_number;
    FILEPARTS * parts;
    std::string md5;

    ~FILESTATS() {
        //delete[] parts;
        //printf("FILESTATS DELETED: %s\n", md5.c_str());
    }
};

int parseFile(Connection *c, char * buf, int length);
std::string md5_file(std::string path);
std::string md5(unsigned char * buffer);
std::string build_file_header(std::string file, int chunk_size, FILESTATS &f);
int calculate_chunk_number(std::string file) ;
int get_file_size(std::string file);
unsigned char * get_chunk_data(std::string file, int chunk_number, int &data_size);
char * build_chunk_header(std::string file, std::string file_md5, int chunk_number, FILEPARTS &f);
int send_chunk_patch(Connection *c, std::string file, std::string super_md5, int chunk_number);
int send_file(Connection *c, std::string file);
std::string get_chunk_md5(std::string file, int chunk_number);
int append_to_file(std::string file, char * buf, int length);
FILEPARTS * get_chunk_from_header(char * header, int length);
FILESTATS * get_super_header(char * header);
int init_file(std::string file, int size);
int write_chunk_to_file(std::string file, char * buf, int length, int chunk_number, int chunk_size);

#endif