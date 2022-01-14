#ifndef USER_H
#define USER_H

#include <stdio.h>

#define DEFAULT_PORT 58000
#define IPV4_SIZE 12
#define GROUP_NUMBER 62
#define SIZE_STRING 128
#define SIZE_GROUP_NAMES 24
#define SIZE_GROUPS 3273
#define COM_SIZE 4
#define MAX_GROUPS 99
#define MAX_TEXT_SIZE 242


extern char* DSIP;
extern char* DSport;
extern char* currentID;
extern char* currentPass;
extern bool isLoggedIn;

int TimerON(int sd);
int TimerOFF(int sd);
void verifyArguments(int numArgs, char *args[]);
void getDefaultPort();
void getDefaultIP();
void parseArgs(int numArgs,char *args[]);
char* clientSendUDP(char* message, int sizeString);
void fileSendTCP(char* filename, long fsize, int fd);
char* clientSendTCP(char* message, char* fileName, long fsize);
char* verifyFile(char fileName[], long* fsize);
void commandRegister(char* message);
void commandUnregister(char* message, char* arg1);
void commandLogin(char* message, char* UID, char* pass);
void commandLogout(char* message, char* UID, char* pass);
void commandShowUID();
void commandGroups(char* message);
void commandSubscribe(char* message);
void commandUnsubscribe(char* message);
void commandSelect(char* GID);
void commandShowGID();
void commandMyGroups(char* message);
void commandUList(char* message);
void commandPost(char* command);
int getDigits(int m);
void commandRetrieve(char* command);
void commandExit(char* message);
void processCommands();

#endif 