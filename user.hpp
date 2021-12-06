#ifndef USER_H
#define USER_H

#include <stdio.h>

#define DEFAULT_PORT 58000
#define IPV4_SIZE 12
#define GROUP_NUMBER 62
#define SIZE_STRING 128



extern char* DSIP;
extern char* DSport;

void getDefaultPort();
void getDefaultIP();
void parseArgs(int numArgs,char *args[]);
void clientSend(char* message);
void processCommands();
void comRegister(int UID, char* pass);
void comUnregister();
void comLogin();
void comLogout();
void comShowUID();
void comExit();


#endif 