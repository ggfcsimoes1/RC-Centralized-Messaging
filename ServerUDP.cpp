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
#include <dirent.h>


using namespace std;
bool verifyPassword(char*fileDirectory, char* password,int UID,char* buffer){
	FILE *fp;
	bool verified=false;
	char* pass=(char*)malloc(sizeof(char)*9);

	sprintf(fileDirectory,"USERS/%d/%d_pass.txt",UID ,UID);

	if((fp=fopen(fileDirectory,"r"))==NULL){
		sprintf(buffer, "ERR\n");
		return verified;
	}

	fscanf(fp,"%s\n",pass);

	if(strcmp(pass,password)==0)
		verified=true;

	fclose(fp);
	free(pass);

	return verified;

}
bool logout(int UID){

	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);//HARDCODEDs
	bool sucess = false;

	sprintf(fileDirectory, "USERS/%d/%d_login.txt", UID, UID);

	int rm = remove(fileDirectory);

	if(rm == 0 || errno==ENOENT){
		sucess = true;
	}

	free(fileDirectory);
	return sucess;
}

bool isLoggedIn(char* fileDirectory){
	//se nao usarmos em mais lado criar ficheiro aqui
	FILE* fp;
	
	int errcode = true;

	

	if((fp=fopen(fileDirectory, "r")) == NULL){

		errcode = false;
	}
	else
		fclose(fp);

	
	
	return errcode;
}

void comRegister(char* buffer, int UID, char* pass){
	
	FILE *fp;
	char* directory = (char*) malloc(sizeof(char)* 12);
	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);//HARDCODEDs

	//ALDRABADO
	if ((UID/100000) != 0 || strlen(pass)!=8){ 
		sprintf(buffer, "RRG NOK\n");
		return;
	}
	
	mkdir("USERS",0700);
	sprintf(directory, "USERS/%d", UID);
	
	if(mkdir(directory,0700)== 0){
		sprintf(fileDirectory,"%s/%d_pass.txt",directory ,UID);
		
		if((fp = fopen(fileDirectory, "w+")) == NULL){
			sprintf(buffer, "ERR\n");
			free(directory);
			free(fileDirectory);
			fclose(fp);
			return;
		}
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

	// DO unsubscribe
	
	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);//HARDCODEDs

	sprintf(buffer, "RUN NOK\n");

	if(verifyPassword(fileDirectory,pass,UID,buffer)){
		if(remove(fileDirectory)==0){
			sprintf(fileDirectory,"USERS/%d", UID);

			if(logout(UID) == false || rmdir(fileDirectory) != 0){
				sprintf(buffer, "ERR\n");
				free(fileDirectory);
				return;
			}

			sprintf(buffer, "RUN OK\n");
		}
		else
			sprintf(buffer, "ERR\n");

		
	}
	

	free(fileDirectory);
}

void comLogin(char* buffer, int UID, char* pass){
	FILE *flog;
	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);//HARDCODEDs
	
	sprintf(buffer, "RLO NOK\n");

	if(verifyPassword(fileDirectory,pass,UID,buffer)){
		sprintf(fileDirectory, "USERS/%d/%d_login.txt", UID, UID);
		
		//file + directory exist, everything is valid...
		

		if(!isLoggedIn(fileDirectory)){
			sprintf(buffer, "RLO OK\n");
			if((flog=fopen(fileDirectory,"w"))==NULL)			
				sprintf(buffer,"RLO ERR\n");
			fclose(flog);

			
		}
			
			
		
		

		
		
	}

	


	
	
	free(fileDirectory);
}

void comLogout(char* buffer, int UID, char* pass){
	FILE *fp;
	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);//HARDCODEDs

	sprintf(buffer, "ROU NOK\n");

	if(verifyPassword(fileDirectory,pass,UID,buffer)){
		if(logout(UID))
			sprintf(buffer, "ROU OK\n");
		else
			sprintf(buffer, "ROU ERR\n");
	}

	
	
	free(fileDirectory);


}

/*void comGroups(char* buffer){
	DIR *d;
	struct dirent *dir;
	int i=0;
	FILE *fp;
	char GIDname[30];
	
	list->no_groups=0;
	d = opendir("GROUPS");
	if (d)	{
		while ((dir = readdir(d)) != NULL)	{
			if(dir->d_name[0]=='.')
				continue;
			if(strlen(dir->d_name)>2)
				continue;
			strcpy(list->group_no[i], dir->d_name);
			sprintf(GIDname,"GROUPS/%s/%s_name.txt",dir->d_name,dir->d_name);
			fp=fopen(GIDname,"r");
			if(fp)	{
				fscanf(fp,"%24s",list->group_name[i]);
				fclose(fp);
			}
			++i;
			if(i==99)
				break;
		
		}
		list->no_groups=i;
		closedir(d);
	}
	else
		return(-1);
	if(list->no_groups > 1)
		SortGList(list);
	return(list->no_groups);
}*/

char* processCommands(char* command){
    char *com,*pass,*buffer;
    com=(char*) malloc(sizeof(char)*SIZE_STRING);
    pass=(char*) malloc(sizeof(char)*SIZE_STRING);
    buffer=(char*) malloc(sizeof(char)*SIZE_STRING);
    int n,UID;
  
    n=sscanf(command,"%s %d %s\n",com,&UID,pass); 
        
    if(n<1)
		sprintf(buffer, "ERR\n");
        

    if(strcmp(com,"REG")==0 ){
		if(n==3)
        	comRegister(buffer,UID,pass);
		else
			sprintf(buffer, "ERR\n");
    }
            
    else if(strcmp(com,"UNR")==0){
		if(n==3)
        	comUnregister(buffer,UID,pass);
		else
			sprintf(buffer, "ERR\n");
	}
        
    else if(strcmp(com,"LOG")==0){
		if(n==3)
        	comLogin(buffer,UID,pass);
		else
			sprintf(buffer, "ERR\n");
	}

    else if(strcmp(com,"OUT")==0){
		if(n==3)
        	comLogout(buffer,UID,pass);
		else
			sprintf(buffer, "ERR\n");
	}

	/*else if(strcmp(com,"GLS")==0){
		if(n==1)
        	//comGroups(buffer);
		else
			sprintf(buffer, "ERR\n");
	}*/

	

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





