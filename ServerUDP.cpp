#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <list>
#include <iterator>
#include <algorithm>

#include "ServerUDP.hpp"



using namespace std;

void comRegister(char* buffer, int UID, char* pass){
	
	FILE *fp;
	char* directory = (char*) malloc(sizeof(char)* 12);
	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);//HARDCODED
	char* path = (char*) malloc(sizeof(char)*SIZE_STRING);

	sprintf(path, "USERS/%d", UID);

	if ((UID/100000) != 0) 
		sprintf(buffer, "RRG NOK\n");
	else if(!CFileFind::FindFile(path, 0)){

			if(mkdir("USERS",0755) != 0) 
				sprintf(buffer, "ERR\n");

			sprintf(directory, "USERS/%d", UID); 

			if(mkdir(directory,0755) != 0) 
				sprintf(buffer, "ERR\n");

			sprintf(fileDirectory,"%s/%d_%s.txt",directory ,UID, pass);

			if((fp = fopen(fileDirectory, "w+")) == NULL)
				sprintf(buffer, "ERR\n");

			
			fprintf(fp,"%s\n",pass);
			
			fclose(fp);
			
			sprintf(buffer, "RRG OK\n");
			
			
	}	
	else if(CFileFind::FindFile(path, 0)) //UID in list, duplicate found
			sprintf(buffer, "RRG DUP\n");
	else sprintf(buffer, "RRG NOK\n");
	free(directory);
	free(fileDirectory);
	
}
void comUnregister(){
    return;
}
void comLogin(){
    return;
}
void comLogout(){
    return;
}

char* processCommands(char* command){
    char *com,*pass,*buffer;
    com=(char*) malloc(sizeof(char)*SIZE_STRING);
    pass=(char*) malloc(sizeof(char)*SIZE_STRING);
    buffer=(char*) malloc(sizeof(char)*SIZE_STRING);
    int n,UID;
  
    n=sscanf(command,"%s %d %s",com,&UID,pass); 
        
    if(n<1)
		sprintf(command, "ERR\n");
        

    if(strcmp(com,"REG")==0 && n==3){
        comRegister(buffer,UID,pass);
    }
            
    else if(strcmp(com,"UNR")==0)
        comUnregister();
        
    else if(strcmp(com,"LOG")==0)
        comLogin();

    else if(strcmp(com,"LOU")==0)
        comLogout();

    free(com);
    free(pass);
	return buffer;
}



int main(){

	int fd,errcode;
	ssize_t n;
	socklen_t addrlen;
	struct addrinfo hints,*res;
	struct sockaddr_in addr;
	char buffer[128];
	
	char host[NI_MAXHOST],service[NI_MAXSERV];

	fd=socket(AF_INET,SOCK_DGRAM,0); //UDP Socket
	if (fd==1) //error
		exit(1);

	memset(&hints,0,sizeof hints);
	hints.ai_family=AF_INET; //IPV4
	hints.ai_family=SOCK_DGRAM; //UDP socket
	hints.ai_flags=AI_PASSIVE;

	errcode=getaddrinfo(NULL, PORT, &hints, &res);
	if(errcode!=0) //error
		exit(1);

	n=bind(fd,res->ai_addr, res->ai_addrlen);
	if(n==-1)
		exit(1);

	while(1){
		addrlen=sizeof(addr);
		n=recvfrom(fd,buffer,128,0,(struct sockaddr*) &addr, &addrlen);
		if(n==-1)
			exit(1);
		if((errcode=getnameinfo((struct sockaddr*)&addr,addrlen,host,sizeof host,service,sizeof service,0))!=0)
			fprintf(stderr,"error:getnameinfo: %s\n",gai_strerror(errcode));
		else
			printf("sent by [%s:%s]\n",host,service);

		processCommands(buffer);
	
		//strcpy(buffer, "amo te barbara tinoco\n");
		n=sendto(fd,buffer,strlen(buffer),0, (struct sockaddr*) &addr, addrlen);
		if(n==-1)
			exit(1);


	}
	freeaddrinfo(res);
	close(fd);
	exit(0);
}





