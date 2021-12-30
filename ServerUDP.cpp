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

int currentGroups=0; //number of groups in the directory


void getNumberOfGroups(){
	DIR *d;
	struct dirent *dir;
	d = opendir("GROUPS");

	if (d)	{
		while ((dir = readdir(d)) != NULL)	{
			if(dir->d_name[0]=='.')
				continue;
			if(strlen(dir->d_name)>2)
				continue;
			++currentGroups;
			if(currentGroups==99)
				break;	
		}
		printf("no of groups %d\n", currentGroups);
		closedir(d);
	}
}

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
	bool success = false;

	sprintf(fileDirectory, "USERS/%d/%d_login.txt", UID, UID);

	int rm = remove(fileDirectory);

	if(rm == 0 || errno==ENOENT){
		success = true;
	}

	free(fileDirectory);
	return success;
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
		free(directory);
		free(fileDirectory);
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

void comGroups(char* buffer){ //implementar messageID
	DIR *d;
	struct dirent *dir;
	int i=0;
	FILE *fp;
	char GIDname[30];

	
	
	d = opendir("GROUPS");

	sprintf(buffer, "RGL %d", currentGroups);

	if (d)	{
		while ((dir = readdir(d)) != NULL)	{
			if(dir->d_name[0]=='.')
				continue;
			if(strlen(dir->d_name)>2)
				continue;

			sprintf(GIDname,"GROUPS/%s/%s_name.txt",dir->d_name,dir->d_name);
			fp=fopen(GIDname,"r");
			if(fp)	{
				fscanf(fp,"%24s",GIDname);
				fclose(fp);
			}
			sprintf(buffer, "%s %s %s", buffer,dir->d_name, GIDname);
			++i;
			if(i==99)
				break;
		
		}
		closedir(d);
		sprintf(buffer, "%s\n", buffer);
	}
	else
		sprintf(buffer, "ERR\n");
	
}

void comSubscribe(char* buffer, int UID, char* GID, char* GNAME){
	
	FILE *fp;
	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);

	printf("%d\n", atoi(GID));

	sprintf(fileDirectory, "USERS/%d/%d_pass.txt", UID, UID);

	if((fp = fopen(fileDirectory, "r")) == NULL){
		sprintf(buffer, "RGS E_USR\n");
		return;
	}
	else if(strlen(GID) != 2 || !isdigit(GID[0]) || !isdigit(GID[1]) ||atoi(GID) > currentGroups || atoi(GID) < 0){
		sprintf(buffer, "RGS E_GRP\n");
		return;
	}
	else if(strlen(GNAME) > 24){
		for(int i= 0; i < strlen(GNAME); i++){
			if(!isalpha(GNAME[i]) && !isdigit(GNAME[i]) && GNAME[i] != '_' && GNAME[i] != '-'){
				sprintf(buffer, "RGS E_GNAME\n");
				return;
			}
		}
	}

	fclose(fp);

	
	if(strcmp("00", GID) == 0 && currentGroups < 99){ //create a new group...
		
		mkdir("GROUPS", 0700);
		currentGroups++; //increment group number
		sprintf(fileDirectory, "GROUPS/%02d", currentGroups);
		if(mkdir(fileDirectory, 0700)!= 0){
			sprintf(buffer, "ERR\n");
			free(fileDirectory);
			return;
		}

		sprintf(fileDirectory, "GROUPS/%02d/MSG", currentGroups);
		if(mkdir(fileDirectory, 0700)!= 0){
			sprintf(buffer, "ERR\n");	
			free(fileDirectory);
			return;
		}
	
		sprintf(fileDirectory, "GROUPS/%02d/%02d_name.txt", currentGroups, currentGroups);
		printf("dir %s\n", fileDirectory);
		if((fp = fopen(fileDirectory, "w+")) == NULL){
			sprintf(buffer, "ERR\n");
			free(fileDirectory);
			return;
		}

		fprintf(fp,"%s\n", GNAME);

		fclose(fp);

		sprintf(fileDirectory, "GROUPS/%02d/%d.txt", currentGroups, UID);
		if((fp = fopen(fileDirectory, "w+")) == NULL){
			sprintf(buffer, "ERR\n");
			free(fileDirectory);
			return;
		}

		sprintf(buffer, "RGS NEW %02d\n", currentGroups);
	
		fclose(fp);
		free(fileDirectory);
		return;
	} 
	else if (strcmp("00", GID) != 0){ //subscribe to an existing group, check if they're valid digits and that the group exists
		sprintf(fileDirectory, "GROUPS/%02d/%d.txt", currentGroups, UID);
		if((fp = fopen(fileDirectory, "w+")) == NULL){
			sprintf(buffer, "ERR\n");
			free(fileDirectory);
			return;
		}
	} 
	else if (strcmp("00",GID)==0) //when there's 99 groups registered already (directory is full)
		sprintf(buffer, "E_FULL");
}

void comUnsubscribe(char* buffer, int UID, char* GID){

	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);
	sprintf(fileDirectory, "GROUPS/%02d/%d.txt", currentGroups, UID);

	if(strlen(GID) != 2 || !isdigit(GID[0]) || !isdigit(GID[1]) ||atoi(GID) > currentGroups || atoi(GID) < 0){ // check if GID is valid
		sprintf(buffer, "RGU E_GRP\n");
		free(fileDirectory);
		return;
	}
	
	int removeStatus = remove(fileDirectory);
	if(removeStatus == 0){ //if it was able to remove
		sprintf(buffer, "RGU OK\n");
	}
	else if (removeStatus != 0 && errno==ENOENT){ //user wasn't registered (invalid UID)
		sprintf(buffer, "RGU E_USR\n");
	}
	else{
		sprintf(buffer, "RGU NOK\n");
	}
	free(fileDirectory);
	return;
}

char* processCommands(char* command){
	char* groupNames;
    char* com = (char*) malloc(sizeof(char)*SIZE_STRING);
    char* arg2=(char*) malloc(sizeof(char)*SIZE_STRING);
	char* arg3=(char*) malloc(sizeof(char)*SIZE_GROUP_NAMES);
    char* buffer=(char*) malloc(sizeof(char)*SIZE_STRING);
    int n,arg1;
	n=sscanf(command, "%[^ ] %d %[^ ] %[^ ]\n",com, &arg1, arg2, arg3);
    printf("%d\n",n);
	

    if(n<1)
		sprintf(buffer, "ERR\n");
        

    if(strcmp(com,"REG")==0 ){
		
		if(n==3){
			comRegister(buffer,arg1,arg2);
		}
		else
			sprintf(buffer, "ERR\n");
    }
            
    else if(strcmp(com,"UNR")==0){
		
		if(n==3)
        	comUnregister(buffer,arg1,arg2);
		else
			sprintf(buffer, "ERR\n");
	}
        
    else if(strcmp(com,"LOG")==0){
		
		if(n==3)
        	comLogin(buffer,arg1,arg2);
		else
			sprintf(buffer, "ERR\n");
	}

    else if(strcmp(com,"OUT")==0){
		
		if(n==3)
        	comLogout(buffer,arg1,arg2);
		else
			sprintf(buffer, "ERR\n");
	}

	else if(strcmp(com,"GLS")==0){
		if(n==1){
			groupNames = (char*) malloc(sizeof(char)*(7 + (currentGroups)*(24 + 4)));
        	comGroups(groupNames);
			free(com);
			free(arg2);
			free(arg3);
			return groupNames;
		}
		else
			sprintf(buffer, "ERR\n");
	}

	else if(strcmp(com,"GSR")==0){
		
		if(n==4){
			comSubscribe(buffer,arg1,arg2,arg3);
		}
		else
			sprintf(buffer, "ERR\n");
	}

	else if(strcmp(com,"GUR")==0){
		
		if(n==3){
			comUnsubscribe(buffer,arg1,arg2);
		}
		else
			sprintf(buffer, "ERR\n");
	}	

    free(com);
    free(arg2);
	free(arg3);
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


	getNumberOfGroups();

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
		else{ //se tiver em modo verboso da print disto
			printf("sent by [%s:%s]\n",host,service);
			//dar print no process commands do GID e UID, e a descrição do comando
		}
		buffer2 = processCommands(buffer);
		
		n=sendto(fd,buffer2,strlen(buffer2),0, (struct sockaddr*) &addr, addrlen);
		if(n==-1)
			exit(1);


	}
	freeaddrinfo(res);
	close(fd);
	exit(0);
}





