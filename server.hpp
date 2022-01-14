#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>

#define SIZE_STRING 128
#define SIZE_GROUP_NAMES 24
#define PORT "58062"
using namespace std;

bool isUserSub(char* uid, char* gid);
bool verifyIDs(char* ID, int len);
bool verifyPassword(char* pass);
bool verifyUID(char* uid);
bool verifyGID(char* gid);
bool verifyMID(char* MID,char* gid);
bool userLogged(char* uid);
int setSocketUDP();
int setSocketTCP();
int receiveUDP(int fd);
void receiveTCP(int fd);
void sendTCP(char* buffer, int fd);
void sendFileTCP(FILE *fp, int fd, int fsize);
void getNumberOfGroups();
bool validatePassword(char* password, char* UID);
bool logout(char* UID);
int getNumberEntInDir(char* fileDir);
void comRegister(char* buffer, char* UID, char* pass);
void comUnregister(char* buffer, char* UID, char* pass);
void comLogin(char* buffer, char* UID, char* pass);
void comLogout(char* buffer, char* UID, char* pass);
void comGroups(char* buffer);
bool createGroup(char *UID, char* GNAME);
void comSubscribe(char* buffer, char* UID, char* GID, char* GNAME);
void comUnsubscribe(char* buffer, char* UID, char* GID);
void comMyGroups(char* buffer,char* uid);
void comUList(char* buffer,char* gid);
char* getFileName(char* fileDir);
void addMSG(char* fileDir, int msg, char* uid, char* text, int tsize);
void addExtraFile(int fd, char* fileDir, int msg);
void comPost(int fd);
int getMsgToSend(int m[], char* gid, int nMid);
void comRetrieve(char* uid, char* gid, char* mid, int fd);
char* processCommands(char* command, int fd);

#endif