#ifndef SERVERUDP_H
#define SERVERUDP_H

#include <stdio.h>

#define SIZE_STRING 128

void comRegister(int UID, char* pass);
void comUnregister();
void comLogin();
void comLogout();
void comShowUID();
void comExit();
void processCommands(char* command);

#endif