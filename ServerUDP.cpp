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
#include <errno.h>
#include "ServerUDP.hpp"


using namespace std;

void comRegister(char* buffer, int UID, char* pass){
	
	FILE *fp;
	char* directory = (char*) malloc(sizeof(char)* 12);
	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);//HARDCODEDs

	if ((UID/100000) != 0 || strlen(pass)!=8){ 
		sprintf(buffer, "RRG NOK\n");
		return;
		
	}
	
	mkdir("USERS",0755);
	sprintf(directory, "USERS/%d", UID);
	
	if(mkdir(directory,0755)== 0){
		sprintf(fileDirectory,"%s/%d_%s.txt",directory ,UID, pass);
		
		if((fp = fopen(fileDirectory, "w+")) == NULL)
			sprintf(buffer, "ERR\n");

		fprintf(fp,"%s\n",pass);
	
		fclose(fp);
	
		sprintf(buffer, "RRG OK\n");

	}
	else if(errno==EEXIST){
		sprintf(buffer, "RRG DUP\n");
	}
	else {
		sprintf(buffer, "RRG NOK\n");
	}
	
	free(directory);
	free(fileDirectory);
	
}
void comUnregister(char* buffer, int UID, char* pass){

	FILE *fp;
	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);//HARDCODEDs

	sprintf(fileDirectory,"USERS/%d/%d_%s.txt",UID,UID, pass);

	if((fp = fopen(fileDirectory, "r+")) == NULL && errno == EEXIST){
	//file + directory exist, everything is valid...
			//commandUnsub();
			sprintf(buffer, "UNR OK\n");
	}
	else if((fp = fopen(fileDirectory, "r+")) == NULL && errno != EEXIST){
			sprintf(buffer, "UNR NOK\n");
	}
	else {
		sprintf(buffer, "ERR\n");
	}
	free(fileDirectory);

}
void comLogin(char* buffer, int UID, char* pass){
	FILE *fp;
	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);//HARDCODEDs

	sprintf(fileDirectory,"USERS/%d/%d_%s.txt",UID,UID, pass);

	if((fp = fopen(fileDirectory, "r+")) == NULL && errno == EEXIST){
	//file + directory exist, everything is valid...
			sprintf(buffer, "RLO OK\n");
	}
	else if((fp = fopen(fileDirectory, "r+")) == NULL && errno != EEXIST){
			sprintf(buffer, "RLO NOK\n");
	}
	else {
		sprintf(buffer, "ERR\n");
	}
	free(fileDirectory);


}
void comLogout(char* buffer, int UID, char* pass){
	FILE *fp;
	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);//HARDCODEDs

	sprintf(fileDirectory,"USERS/%d/%d_%s.txt",UID,UID, pass);

	if((fp = fopen(fileDirectory, "r+")) == NULL && errno == EEXIST){
	//file + directory exist, everything is valid...
			sprintf(buffer, "ROU OK\n");
	}
	else if((fp = fopen(fileDirectory, "r+")) == NULL && errno != EEXIST){
			sprintf(buffer, "ROU NOK\n");
	}
	else {
		sprintf(buffer, "ERR\n");
	}
	free(fileDirectory);


}

char* processCommands(char* command){
    char *com,*pass,*buffer;
    com=(char*) malloc(sizeof(char)*SIZE_STRING);
    pass=(char*) malloc(sizeof(char)*SIZE_STRING);
    buffer=(char*) malloc(sizeof(char)*SIZE_STRING);
    int n,UID;
  
    n=sscanf(command,"%s %d %s\n",com,&UID,pass); 
        
    if(n<1)
		sprintf(buffer, "ERR\n");
        

    if(strcmp(com,"REG")==0 && n==3){
        comRegister(buffer,UID,pass);
    }
            
    else if(strcmp(com,"UNR")==0)
        comUnregister(buffer,UID,pass);
        
    else if(strcmp(com,"LOG")==0)
        comLogin(buffer,UID,pass);

    else if(strcmp(com,"LOU")==0)
        comLogout(buffer,UID,pass);

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
	char * buffer2;
	
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

		buffer2 = processCommands(buffer);
	
		//strcpy(buffer, "amo te barbara tinoco\n");
		n=sendto(fd,buffer2,strlen(buffer),0, (struct sockaddr*) &addr, addrlen);
		if(n==-1)
			exit(1);


	}
	freeaddrinfo(res);
	close(fd);
	exit(0);
}





