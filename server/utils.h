#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>


char* itoa(int value, char* result, int base);
void error(const char *msg);
void errorLite(const char *msg);