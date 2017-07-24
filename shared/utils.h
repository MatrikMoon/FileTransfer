#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <mutex>
#include <fstream>


char* itoa(int value, char* result, int base);
void error(const char *msg);
void errorLite(const char *msg);
bool file_exists(const std::string& name);