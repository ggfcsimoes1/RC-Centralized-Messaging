#ifndef USER_H
#define USER_H

#include <stdio.h>

#define DEFAULT_PORT 58000
#define IPV4_SIZE 12
#define GROUP_NUMBER 62
#define SIZE_STRING 128

extern char* DSIP;
extern char* DSport;
extern char* currentID;
extern char* currentPass;
extern bool isLoggedIn;

void getDefaultPort();
void getDefaultIP();
void parseArgs(int numArgs,char *args[]);
char* clientSend(char* message);
void commandRegister(char* message);
void commandUnregister(char* message);
void commandLogin(char* message, char* UID, char* pass);
void commandLogout(char* message, char* UID, char* pass);
void commandShowUID();
void processCommands();

#endif 