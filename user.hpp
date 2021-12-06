#ifndef USER_H
#define USER_H

#include <stdio.h>

#define DEFAULT_PORT 58000
#define IPV4_SIZE 12
#define GROUP_NUMBER 12

extern char* DSIP;
extern int DSport;

void getDefaultPort();
void getDefaultIP();
void parseArgs(int numArgs,char *args[]);


#endif 