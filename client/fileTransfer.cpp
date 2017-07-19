#include "fileTransfer.h"

bool file_send_active = false;

int parseFile(Server *c, std::string recv) {
    //Ensure we don't waste processing time if it's not a file
    if (strncmp(recv.c_str(), "/file", 5) != 0) {
        return 0;
    }
    if (strncmp(recv.c_str(), "/file-test", 10) == 0) {
        file_send_active = true;
    }
    else if (strncmp(recv.c_str(), "/end_file", 9) == 0) {

    }
    return 0;
}

#define BUFFER_SIZE 16
unsigned char buffer[BUFFER_SIZE];
int file_read(std::string path) {
    std::ifstream input_file("test.txt", std::ifstream::binary);
    while (input_file.read((char *)buffer, BUFFER_SIZE))
    {
        printf("%s\n", (char*)buffer);
        bzero(buffer, BUFFER_SIZE);
        //output_file.write((char *)buffer, BUFFER_SIZE);
    }
    input_file.close();
}