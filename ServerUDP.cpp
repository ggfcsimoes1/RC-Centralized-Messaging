#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#include "ServerUDP.hpp"

#define PORT "58001"

void comRegister(int UID, char* pass){
    char* command=(char*)malloc(sizeof(char)*SIZE_STRING);
    
    
    free(command);
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
void comShowUID(){
    return;
}
void comExit(){
    return;
}

void processCommands(char* command){
    char *com,*pass;
    com=(char*) malloc(sizeof(char)*SIZE_STRING);
    pass=(char*) malloc(sizeof(char)*SIZE_STRING);
    


    int n,UID;
    
        
        
    n=sscanf(command,"%s %d %s",com,&UID,pass); 
        
    if(n<1)
        return;
        //printf("%s %d %s\n",com,UID,pass);

    if(strcmp(com,"reg")==0){
            
        comRegister(UID,pass);
            
    }
            

    else if(strcmp(com,"unregister")==0 || strcmp(com,"unr")==0)
        comUnregister();
        
    else if(strcmp(com,"login")==0)
        comLogin();

    else if(strcmp(com,"logout")==0)
        comLogout();

    else if(strcmp(com,"showuid")==0 || strcmp(com,"su")==0)
        comShowUID();

    else if(strcmp(com,"exit")==0)
        comExit();

        
        
        
        
        
        
    
    free(com);
    free(pass);
	


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
		write(1,"received: ",10);
		write(1,buffer,n);
		strcpy(buffer, "amo te barbara tinoco\n");
		n=sendto(fd,buffer,strlen(buffer),0, (struct sockaddr*) &addr, addrlen);
		if(n==-1)
			exit(1);


	}
	freeaddrinfo(res);
	close(fd);
	exit(0);
}





