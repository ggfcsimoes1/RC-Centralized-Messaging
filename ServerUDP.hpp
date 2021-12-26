#ifndef SERVERUDP_H
#define SERVERUDP_H

#include <stdio.h>
#include <list>

#define SIZE_STRING 128
#define PORT "58062"
using namespace std;

extern list <int> registeredUsers;

void comRegister(char* command, int UID, char* pass);
void comUnregister(char* buffer, int UID, char* pass);
void comLogin(char* buffer, int UID, char* pass);
void comLogout(char* buffer, int UID, char* pass);
void comExit();
char* processCommands(char* command);
//void comGroups(char* buffer);

#endif