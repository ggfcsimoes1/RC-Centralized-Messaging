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
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include "ServerUDP.hpp"



using namespace std;

int currentGroups = 0; //number of groups in the directory
bool verboseMode = false;


bool isUserSub(char* uid, char* gid){
	FILE *fp;
	char* buffer=(char*) malloc(sizeof(char)*20);
	sprintf(buffer,"GROUPS/%s/%s.txt",gid,uid);
	if(fp=fopen(buffer,"r")){
		fclose(fp);
		free(buffer);
		return true;
	}
	free(buffer);
	return false;
}
bool verifyIDs(char* ID, int len){
	int i=0;
	int n=strlen(ID);
	if(n!=len)
		return false;
	while(i<n){
		if(!isdigit(ID[i]))
			return false;
		i++;
	}
	
	return true;
}

bool verifyPassword(char* pass){
	int i=0;
	int n=strlen(pass);
	if(n!= 8)
		return false;
	while(i<n){
		if(!isdigit(pass[i]) && !isalpha(pass[i]))
			return false;
		i++;
	}
	
	return true;
}


bool verifyUID(char* uid){
	char* buffer;
	DIR *d;
	if(verifyIDs(uid,5)){
		buffer=(char*)malloc(sizeof(char)*12);
		sprintf(buffer,"USERS/%s",uid);
		if(d=opendir(buffer)){
			closedir(d);
			free(buffer);
			return true;
		}	
		else{
			free(buffer);
			return false;
		}
			
	}
	else
		return false;
	
}
bool verifyGID(char* gid){
	char* buffer;
	DIR *d;
	if(verifyIDs(gid,2)){
		buffer=(char*)malloc(sizeof(char)*10);
		sprintf(buffer,"GROUPS/%s",gid);
		if(d=opendir(buffer)){
			closedir(d);
			free(buffer);
			return true;
		}
		else{
			free(buffer);
			return false;
		}
	}
	else
		return false;
}
bool verifyMID(char* MID,char* gid){
	DIR *d;
	char* buffer;
	if(verifyIDs(MID,4)){
		buffer=(char*)malloc(sizeof(char)*12);
		sprintf(buffer,"GROUPS/%s/MSG/%s",gid,MID);
		if(d=opendir(buffer)){
			closedir(d);
			free(buffer);
			return true;
		}
		else{
			free(buffer);
			return false;
		}
			
	}
	else
		return false;
}

bool userLogged(char* uid){
	FILE *fp;
	char* buffer = (char*) malloc(sizeof(char) * SIZE_STRING);

	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "USERS/%s/%s_login.txt", uid, uid);

	if((fp = fopen(buffer, "r")) == NULL){
		free(buffer);
		return false;
	}

	fclose(fp);
	free(buffer);
	return true;
}

int setSocketUDP(){
	int fd,errcode;
	ssize_t n;
	struct addrinfo *res;
	socklen_t addrlen;
	struct addrinfo hints;
	
	fd=socket(AF_INET,SOCK_DGRAM,0); //UDP Socket
	if (fd==-1) //error
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
	
	freeaddrinfo(res);
	return fd;
}

int setSocketTCP(){
	int fd, errcode;
    ssize_t n;
	struct addrinfo *res;
    socklen_t addrlen;
    struct addrinfo hints;
    struct sockaddr_in addr;
    char buffer[128];
	char * buffer2;
    

    fd = socket(AF_INET, SOCK_STREAM,0);
    if(fd==-1) 
		exit(1);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    errcode = getaddrinfo(NULL, PORT, &hints, &res);
    if((errcode)!= 0) 
		exit(1);

    n = bind(fd, res->ai_addr, res->ai_addrlen);
    if(n == -1) 
		exit(1);

    if(listen(fd,5) == -1) 
		exit(1);

	freeaddrinfo(res);
	return fd;
}

int receiveUDP(int fd){
	int errcode;
	ssize_t n;
	socklen_t addrlen;
	struct sockaddr_in addr;
	char buffer[128];
	char * buffer2;
	char host[NI_MAXHOST],service[NI_MAXSERV];

	addrlen=sizeof(addr);
	n=recvfrom(fd,buffer,128,0,(struct sockaddr*) &addr, &addrlen);
	if(n==-1)
		return -1;
	bzero(buffer + n, sizeof(buffer + n));
	if((errcode=getnameinfo((struct sockaddr*)&addr,addrlen,host,sizeof host,service,sizeof service,0))!=0)
		fprintf(stderr,"error:getnameinfo: %s\n",gai_strerror(errcode));
	else if (verboseMode){ //if the server is running with -v
		printf("sent by [%s:%s]\n",host,service);
		//dar print no process commands do GID e UID, e a descricao do comando
	}

	buffer2 = processCommands(buffer, 0);
	
	n=sendto(fd,buffer2,strlen(buffer2),0, (struct sockaddr*) &addr, addrlen);
	if(n==-1)
		return -1;

	return 0;
}

void receiveTCP(int fd){
    int errcode, i = 1;
    ssize_t n;
	long nread = 0;
    //char buffer[11];
    char *message;

	//FILE* fp2;

    message = NULL;

    while(1){
		message =(char*) realloc(message, sizeof(char) * ((i * 64) + 1));

        if(i == 1){

            memset(message, 0, sizeof(message));
			
        }

		n=read(fd, message + nread , 64);

		printf("%ld\n", nread);

		if(n == -1 && errno == EWOULDBLOCK){
        	break;
    	}
		else if(n == -1){
			exit(1);
		}

        nread+= n;
        i++;
    }
	message[nread]='\0'; //tem lixo no fim
	

	//printf("%ld\n", nread);
	
	//fwrite(message,1,nread,fp2);
 
	//fclose(fp2);

	
	

    processCommands(message, fd);
	
	free(message);
}

void sendTCP(char* buffer, int fd){
	ssize_t n, toWrite;
	char *ptr;

	

	ptr = buffer;
    toWrite = strlen(buffer);

    while(toWrite > 0){
        n=write(fd, ptr, toWrite);

        if(n<=0)
            exit(1);

        toWrite-= n;
        ptr+= n;
    }

    free(buffer);
}



void sendFileTCP(FILE *fp, int fd, int fsize){
	char* buffer = (char*) malloc(sizeof(char)*2048);
    long toSend, n1, n2;

    bzero(buffer, 2048); 

    toSend = fsize;
    while(toSend > 0) {
        n1 = fread(buffer, 1, 2048, fp);
        printf("enviado:%ld\n",n1);
        while((n2 = write(fd, buffer, n1)) == -1){}
        
        toSend-= n2;
        bzero(buffer, 2048);
    }

	free(buffer);
}

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
		printf("Registered groups: %d\n", currentGroups); //tira isso, tira isso !
		closedir(d);
	}
}


bool validatePassword(char* password, char* UID){
	FILE *fp;
	bool verified=false;
	char* pass=(char*)malloc(sizeof(char)*9);
	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);//HARDCODEDs

	sprintf(fileDirectory,"USERS/%s/%s_pass.txt",UID ,UID);

	if((fp=fopen(fileDirectory,"r"))==NULL ){
        free(pass);
		free(fileDirectory);
        return verified;
    }

	fscanf(fp,"%s\n",pass);

	if(strcmp(pass,password)==0)
		verified=true;

	fclose(fp);
	free(pass);
	free(fileDirectory);

	return verified;

}
bool logout(char* UID){

	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);//HARDCODEDs
	bool success = false;

	sprintf(fileDirectory, "USERS/%s/%s_login.txt", UID, UID);

	int rm = remove(fileDirectory);

	if(rm == 0 || errno==ENOENT){
		success = true;
	}

	free(fileDirectory);
	return success;
}


int getNumberEntInDir(char* fileDir){
	DIR *d;
	struct dirent *dir;
	int nDir = 0;

	d = opendir(fileDir);

	if (d){
		while ((dir = readdir(d)) != NULL)	{
			if(dir->d_name[0]=='.')
				continue;
			nDir++;
		}
		closedir(d);
		return nDir;
	}
	else{
		return -1;
	}
}

void comRegister(char* buffer, char* UID, char* pass){
	
	FILE *fp;
	char* directory;
	char* fileDirectory; 

	if(!verifyIDs(UID, 5) || !verifyPassword(pass)){
		sprintf(buffer, "RRG NOK\n");
		return;
	}

	directory = (char*) malloc(sizeof(char)* 12);
	fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);//HARDCODEDs

	mkdir("USERS",0700);
	sprintf(directory, "USERS/%s", UID);
	
	if(mkdir(directory,0700)== 0){
		sprintf(fileDirectory,"%s/%s_pass.txt",directory ,UID);
		
		if((fp = fopen(fileDirectory, "w+")) == NULL){
			
			sprintf(directory, "USERS/%s", UID);
			remove(directory);

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

void comUnregister(char* buffer, char* UID, char* pass){

    DIR *d;
    struct dirent *dir;
    int i=0;
    char *GIDname = (char*) malloc(sizeof(char) * 30);
    char *fileDirectory = (char*) malloc(sizeof(char) * SIZE_STRING);//HARDCODEDs

    sprintf(buffer, "RUN NOK\n");

    if(verifyUID(UID) && validatePassword(pass,UID)){
		sprintf(fileDirectory,"USERS/%s/%s_pass.txt",UID ,UID);

        if(remove(fileDirectory)==0){
            sprintf(fileDirectory,"USERS/%s", UID);

            if(logout(UID) == false || rmdir(fileDirectory) != 0){
                free(fileDirectory);
                return;
            }
            d = opendir("GROUPS");
            if (d)    {
                while ((dir = readdir(d)) != NULL){
                    if(dir->d_name[0]=='.')
                        continue;
                    if(strlen(dir->d_name)>2)
                        continue;

                    sprintf(GIDname,"GROUPS/%s/%s.txt",dir->d_name,UID);

                    if(remove(GIDname)!=0 && errno!=ENOENT)
                        sprintf(buffer, "ERR\n");

                    ++i;
                    if(i==99)
                        break;
            }
            closedir(d);
            }

            sprintf(buffer, "RUN OK\n");
        }
        else
            sprintf(buffer, "ERR\n");
    }
    free(fileDirectory);
	free(GIDname);
}

void comLogin(char* buffer, char* UID, char* pass){
	FILE *flog;
	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);
	
	sprintf(buffer, "RLO NOK\n");

	if(verifyUID(UID) && validatePassword(pass,UID) && !userLogged(UID)){

		sprintf(fileDirectory, "USERS/%s/%s_login.txt", UID, UID);
		
		if((flog=fopen(fileDirectory,"w"))==NULL)			
			sprintf(buffer,"RLO NOK\n");
		else{
			sprintf(buffer, "RLO OK\n");
			fclose(flog);
		}
	}

	free(fileDirectory);
}

void comLogout(char* buffer, char* UID, char* pass){

	sprintf(buffer, "ROU NOK\n");

	if(verifyUID(UID) && validatePassword(pass, UID)){
		if(logout(UID))
			sprintf(buffer, "ROU OK\n");
		else
			sprintf(buffer, "ERR\n");
	}
}

void comGroups(char* buffer){
	DIR *d;
	struct dirent *dir;
	int i=0;
	FILE *fp;
	char *GIDname = (char*) malloc(sizeof(char) * 30);
	char *dirName = (char*) malloc(sizeof(char) * 60);
	
	d = opendir("GROUPS");

	sprintf(buffer, "RGL %d", currentGroups);

	if (d)	{
		while ((dir = readdir(d)) != NULL)	{
			if(dir->d_name[0]=='.')
				continue;
			if(strlen(dir->d_name)>2)
				continue;

			sprintf(dirName,"GROUPS/%s/%s_name.txt",dir->d_name,dir->d_name);
			fp=fopen(dirName,"r");

			if(fp!=NULL)	{
				fscanf(fp,"%24s",GIDname);
				fclose(fp);
			}

			sprintf(dirName, "GROUPS/%s/MSG", dir->d_name);

			sprintf(buffer, "%s %s %s %04d", buffer, dir->d_name, GIDname, getNumberEntInDir(dirName));
			++i;
			if(i==99)
				break;
		
		}
		closedir(d);
		sprintf(buffer, "%s\n", buffer);
	}
	else
		sprintf(buffer, "ERR\n");
	
	free(GIDname);
}

bool createGroup(char *UID, char* GNAME){
	FILE* fp;
	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);

	mkdir("GROUPS", 0700);
	currentGroups++; //increment group number

	sprintf(fileDirectory, "GROUPS/%02d", currentGroups);
	if(mkdir(fileDirectory, 0700)== 0){
		sprintf(fileDirectory, "GROUPS/%02d/MSG", currentGroups);
		
		if(mkdir(fileDirectory, 0700)== 0){
			sprintf(fileDirectory, "GROUPS/%02d/%02d_name.txt", currentGroups, currentGroups);
			
			if((fp = fopen(fileDirectory, "w+")) != NULL){
				fprintf(fp,"%s\n", GNAME);

				fclose(fp);

				sprintf(fileDirectory, "GROUPS/%02d/%s.txt", currentGroups, UID);
				
				if((fp = fopen(fileDirectory, "w+")) != NULL){
					fclose(fp);
					free(fileDirectory);
					return true;
				}

				sprintf(fileDirectory, "GROUPS/%02d/%02d_name.txt", currentGroups, currentGroups);
				remove(fileDirectory);
			}
			
			sprintf(fileDirectory, "GROUPS/%02d/MSG", currentGroups);
			remove(fileDirectory);
		}

		sprintf(fileDirectory, "GROUPS/%02d", currentGroups);
		remove(fileDirectory);
	}

	currentGroups--;

	free(fileDirectory);
	return false;
}

void comSubscribe(char* buffer, char* UID, char* GID, char* GNAME){
	
	FILE *fp;
	char fileDirectory1[128];
	char name[25];

	if(!(verifyGID(GID) || strcmp(GID, "00") == 0)){
		sprintf(buffer, "RGS E_GRP\n");
		return;
	}
	else if(!verifyUID(UID)){
		sprintf(buffer, "RGS E_USR\n");
		return;
	}
	else if(!userLogged(UID)){
		sprintf(buffer, "RGS NOK\n");
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
	
	if(strcmp("00", GID) == 0 && currentGroups < 99){ //create a new group...

		if(createGroup(UID, GNAME)){
			sprintf(buffer, "RGS NEW %02d\n", currentGroups);
		}
		else {
			sprintf(buffer, "RGS NOK\n");
		}
	} 
	else if (strcmp("00", GID) != 0){ //subscribe to an existing group, check if they're valid digits and that the group exists
		
		sprintf(fileDirectory1, "GROUPS/%s/%s_name.txt", GID, GID);
		
		if((fp = fopen(fileDirectory1, "r")) == NULL){
			sprintf(buffer, "ERR\n");
			return;
		}
		else{
			fscanf(fp,"%s\n",name);

			if(strcmp(name,GNAME)!=0){
				sprintf(buffer, "RGS E_GNAME\n");
				fclose(fp);
				return;
			}
			
			fclose(fp);
		}

		sprintf(fileDirectory1, "GROUPS/%s/%s.txt", GID, UID);
		if((fp = fopen(fileDirectory1, "w+")) == NULL){
			sprintf(buffer, "ERR\n");
		}
		else{
			sprintf(buffer,"RGS OK\n");
			fclose(fp);
		}

	} 
	else if (currentGroups==99) //when there's 99 groups registered already (directory is full)
		sprintf(buffer, "RGS E_FULL");
}

void comUnsubscribe(char* buffer, char* UID, char* GID){
	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);
	int removeStatus;

	sprintf(fileDirectory, "GROUPS/%s/%s.txt", GID, UID);

	if(!verifyGID(GID)){ // check if GID is valid
		sprintf(buffer, "RGU E_GRP\n");
		free(fileDirectory);
		return;
	}

	if(!verifyUID(UID) || !userLogged(UID) || !isUserSub(UID, GID)){ // check if UID is valid
		sprintf(buffer, "RGU E_USR\n");
		free(fileDirectory);
		return;
	}
	
	removeStatus = remove(fileDirectory);
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

void comMyGroups(char* buffer,char* uid){
	DIR *d;
	struct dirent *dir;
	int i=0,numGroups=0;
	FILE *fp;
	char *GIDname = (char*) malloc(sizeof(char) * 30);
	char* aux=(char*)malloc(sizeof(char)*10000);// -----------------Alterar valor
	char* dirName = (char*) malloc(sizeof(char) * 60);

	if(!verifyUID(uid) || !userLogged(uid)){
		sprintf(buffer, "RGM E_USR\n");
		free(dirName);
		free(aux);
		free(GIDname);
		return;
	}

	memset(aux, 0, sizeof(aux));
	memset(buffer, 0, sizeof(buffer));

	d = opendir("GROUPS");

	if (d)	{
		while ((dir = readdir(d)) != NULL)	{
			if(dir->d_name[0]=='.')
				continue;
			if(strlen(dir->d_name)>2)
				continue;

			memset(dirName, 0, sizeof(dirName));
			memset(GIDname, 0, sizeof(GIDname));

			sprintf(dirName,"GROUPS/%s/%s.txt",dir->d_name,uid);
			fp=fopen(dirName,"r");

			if(fp != NULL)	{
				fclose(fp);

				numGroups++;

				sprintf(dirName,"GROUPS/%s/%s_name.txt",dir->d_name,dir->d_name);
				fp=fopen(dirName,"r");
				
				if(fp)	{
					fscanf(fp,"%24s",GIDname);
					fclose(fp);
				}

				sprintf(dirName, "GROUPS/%s/MSG", dir->d_name);

				sprintf(aux, "%s %s %s %04d", aux, dir->d_name, GIDname, getNumberEntInDir(dirName));	
			}
			
			++i;
			if(i==99)
				break;
		}
		
		if(numGroups==0)
			sprintf(buffer, "RGM %d\n",numGroups );
		else
			sprintf(buffer, "RGM %d%s\n", numGroups,aux);

		closedir(d);
		
	}
	else
		sprintf(buffer, "ERR\n");
	
	free(dirName);
	free(aux);
	free(GIDname);
}

void comUList(char* buffer,char* gid){
	DIR *d;
	struct dirent *dir;
	FILE *fp;
	char GIDname[30], user[6];

	if(!verifyGID(gid)){
		sprintf(buffer, "RUL NOK\n");
		return;
	}

	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "GROUPS/%s", gid);
	d = opendir(buffer);

	memset(buffer, 0, sizeof(buffer));

	if (d)	{
		sprintf(GIDname,"GROUPS/%s/%s_name.txt", gid, gid);
		fp=fopen(GIDname,"r");
		if(fp!=NULL){
			fscanf(fp,"%24s",GIDname);
			sprintf(buffer, "RUL OK %s", GIDname);
			fclose(fp);
		}
		else{
			sprintf(buffer, "ERR\n");
			return;
		}

		while ((dir = readdir(d)) != NULL)	{
			if(dir->d_name[0]=='.')
				continue;
			if(strlen(dir->d_name)!=9)
				continue;
			
			sscanf(dir->d_name, "%[^.]", user);
			sprintf(buffer, "%s %s", buffer, user);
		}

		sprintf(buffer, "%s\n", buffer);

		closedir(d);
	}
	else if(errno == ENOENT)
		sprintf(buffer, "RUL NOK\n");
	else 
		sprintf(buffer, "ERR\n");
}

char* getFileName(char* fileDir){
	DIR *d;
	struct dirent *dir;

	d = opendir(fileDir);

	if (d){
		while ((dir = readdir(d)) != NULL)	{
			if(dir->d_name[0]=='.')
				continue;
			
			if (strcmp(dir->d_name, "A U T H O R.txt") == 0 || strcmp(dir->d_name, "T E X T.txt") == 0)
				continue;
			else
				break;
		}
		closedir(d);
		return dir->d_name;
	}
	return NULL;
}

void addMSG(char* fileDir, int msg, char* uid, char* text, int tsize){
	char textDIR[strlen(fileDir) + 18], authorDIR[strlen(fileDir) + 22];
	FILE* fp;
	
	memset(textDIR, 0, sizeof(textDIR));
	memset(authorDIR, 0, sizeof(authorDIR));

	sprintf(textDIR, "%s/%04d", fileDir, msg);

	mkdir(textDIR,0700);

	sprintf(textDIR, "%s/%04d/T E X T.txt", fileDir, msg);
	sprintf(authorDIR, "%s/%04d/A U T H O R.txt", fileDir, msg);

	fp = fopen(textDIR, "w+");
	fwrite(text, 1, tsize, fp);
	fclose(fp);

	fp = fopen(authorDIR, "w+");
	fwrite(uid, 1, 5, fp);
	fclose(fp);
}

void addExtraFile(char* fileDir, int msg, char* command){
	int n;
	long f;
	char fileName[25], fsize[10];
	FILE* fp;

	memset(fsize, 0, sizeof(fsize));
	memset(fileName, 0, sizeof(fileName));

	n=sscanf(command, "%s %s",fileName, fsize);

	if(n==2){
		f = atoi(fsize);
		command += strlen(fileName) + strlen(fsize) + 2;

		sprintf(fileDir, "%s/%04d/%s", fileDir, msg, fileName);

        fp = fopen(fileDir, "wb"); 
		fwrite(command,1,f,fp);

		fclose(fp);
	}
}

void comPost(char* buffer, char* command){
	char uid[6], gid[3], tsize[4];
	int n, msg;
	char *fileDir, *text, *commandAux;
	
	commandAux = command;

	memset(tsize, 0, sizeof(tsize));
	memset(uid, 0, sizeof(uid));
	memset(gid, 0, sizeof(gid));
	commandAux += 4;

	n=sscanf(commandAux, "%s %s %s", uid, gid, tsize);//--------------------fix response

	if(n < 3){
		sprintf(buffer, "ERR\n");
		return;
	}

	if(!verifyUID(uid) || !verifyGID(gid) || !userLogged(uid)|| !isUserSub(uid, gid)){
		sprintf(buffer, "RPT NOK\n");
		return;
	}

	
	if((n = atoi(tsize)) == 0){
		sprintf(buffer, "ERR\n");
		return;
	}

	text= (char*) malloc(sizeof(char)*(n + 1));

	memset(text, 0, sizeof(text));

	commandAux += (10 + strlen(tsize));
	strncpy(text, commandAux, n);
	commandAux += (n + 1);

	fileDir = (char*) malloc(sizeof(char) * 43);
	sprintf(fileDir, "GROUPS/%s/MSG", gid);

	msg = getNumberEntInDir(fileDir);

	addMSG(fileDir, msg + 1, uid, text, n);
	addExtraFile(fileDir, msg + 1, commandAux);

	sprintf(buffer, "RPT %04d\n", msg +1);
	printf("%s\n",buffer);
}

int getMsgToSend(int m[], char* gid, int nMid){
	int numberOfMSG = 0;
	int lastMID;
	char *fileDir = (char*) malloc(sizeof(char) * 80);
	FILE* fp;

	sprintf(fileDir, "GROUPS/%s/MSG", gid);

	lastMID = getNumberEntInDir(fileDir);

	while(nMid <= lastMID && numberOfMSG < 20 ){
		memset(fileDir, 0, sizeof(fileDir));

		sprintf(fileDir, "GROUPS/%s/MSG/%04d/A U T H O R.txt", gid, nMid);
		
		if((fp = fopen(fileDir, "r")) == NULL){
			nMid++;
			continue;	
		}

		fclose(fp);

		memset(fileDir, 0, sizeof(fileDir));

		sprintf(fileDir, "GROUPS/%s/MSG/%04d/T E X T.txt", gid, nMid);
		
		if((fp = fopen(fileDir, "r")) == NULL){
			nMid++;
			continue;	
		}

		fclose(fp);

		m[numberOfMSG] = nMid;

		nMid++;
		numberOfMSG++;		
	}

	free(fileDir);
	return numberOfMSG;
}

void comRetrieve(char* uid, char* gid, char* mid, int fd){
	int msgRead = 0, n, nMid = atoi(mid);
	int tsize, m[20];
	long fsize;
	char *bufferAux, *text, *fileName, *message;
	char author[6];
	char end[2] = "\n";
	FILE* fp;

	message = (char*) malloc(sizeof(char) * 15);
	memset(message, 0, sizeof(message));

	if(!verifyUID(uid) || !verifyGID(gid) || !verifyMID(mid, gid) || !userLogged(uid) || !isUserSub(uid, gid)){
		strcpy(message, "RRT NOK\n");
		sendTCP(message, fd);
		return;
	}

	//printf("%s\n", command);

	if((msgRead = getMsgToSend(m, gid, nMid)) == 0){
		strcpy(message, "RRT EOF 0\n");
		sendTCP(message, fd);
		return;
	}

	fileName = (char* ) malloc(sizeof(char) * 25);
	bufferAux = (char*) malloc(sizeof(char) * 80);

	sprintf(message, "RRT OK %d", msgRead);

	printf("msg: %s\n", message);
	sendTCP(message, fd);

	for(int i = 0; i < msgRead; i++){

		message = (char*) malloc(sizeof(char) * 340);

		memset(message, 0, sizeof(message));
		memset(bufferAux, 0, sizeof(bufferAux));
		memset(author, 0, sizeof(author));
		memset(fileName, 0, sizeof(fileName));

		sprintf(bufferAux, "GROUPS/%s/MSG/%04d/A U T H O R.txt", gid, m[i]);

		printf("m[i]: %d\n", m[i]);
		
		if((fp = fopen(bufferAux, "r")) != NULL){
			fread(author, 1, 5, fp);
			fclose(fp);
		}

		sprintf(bufferAux, "GROUPS/%s/MSG/%04d/T E X T.txt", gid, m[i]);
		
		if((fp = fopen(bufferAux, "r")) != NULL){

			fseek(fp, 0, SEEK_END); //getting file size
			tsize = ftell(fp); 
			fseek(fp, 0, SEEK_SET); 

			text = (char*) malloc(tsize + 1);
			memset(text, 0, sizeof(text));
			
			fread(text, 1, tsize, fp);
			fclose(fp);
		}

		sprintf(message, " %04d %s %d %.*s", m[i], author, tsize, tsize, text);

		sprintf(bufferAux, "GROUPS/%s/MSG/%04d", gid, m[i]);


		if(getNumberEntInDir(bufferAux) > 2){ //has other files to send
			
			strcpy(fileName,getFileName(bufferAux));

			sprintf(bufferAux, "GROUPS/%s/MSG/%04d/%s", gid, m[i], fileName);
			
			if((fp = fopen(bufferAux, "r")) != NULL){
				fseek(fp, 0, SEEK_END); 
				fsize = ftell(fp); 
				fseek(fp, 0, SEEK_SET); 	

				sprintf(message, "%s / %s %ld ", message, fileName, fsize);

				//printf("msg: %s<\n", message);

				sendTCP(message, fd);
				sendFileTCP(fp, fd, fsize);
				fclose(fp);
			}
		}
		else{
			//printf("msg: %s<\n", message);

			sendTCP(message, fd);
		}

		free(text);
	}

	//sendTCP(end, fd);// send '\n'

	free(fileName);
	free(bufferAux);
}

char* processCommands(char* command, int fd){
	char* buffer2;
    char* com = (char*) malloc(sizeof(char)*SIZE_STRING);
	char* arg1=(char*) malloc(sizeof(char)*SIZE_STRING);
    char* arg2=(char*) malloc(sizeof(char)*SIZE_STRING);
	char* arg3=(char*) malloc(sizeof(char)*SIZE_GROUP_NAMES);
    char* buffer=(char*) malloc(sizeof(char)*SIZE_STRING);
    int n;



    n=sscanf(command,"%s",com); 

    if(n<1)
		sprintf(buffer, "ERR\n");
        
	if(verboseMode) printf("%s",command); //printing the command if the server is running with -v

    if(strcmp(com,"REG")==0 ){
		
		n=sscanf(command, "%s %s %s\n",com, arg1, arg2);

		if(n==3){
			comRegister(buffer,arg1,arg2);
		}
		else
			sprintf(buffer, "ERR\n");
    }
            
    else if(strcmp(com,"UNR")==0){
		n=sscanf(command, "%s %s %s\n",com, arg1, arg2);

		if(n==3)
        	comUnregister(buffer,arg1,arg2);
		else
			sprintf(buffer, "ERR\n");
	}
        
    else if(strcmp(com,"LOG")==0){
		n=sscanf(command, "%s %s %s\n",com, arg1, arg2);

		if(n==3)
        	comLogin(buffer,arg1,arg2);
		else
			sprintf(buffer, "ERR\n");
	}

    else if(strcmp(com,"OUT")==0){
		n=sscanf(command, "%s %s %s\n",com, arg1, arg2);

		if(n==3)
        	comLogout(buffer,arg1,arg2);
		else
			sprintf(buffer, "ERR\n");
	}

	else if(strcmp(com,"GLS")==0){
		
		if(n==1){
			buffer2 = (char*) malloc(sizeof(char)*(7 + (currentGroups)*(24 + 4)));// TEST WITH 99 GROUPS
        	comGroups(buffer2);

			free(com);
			free(arg1);
			free(arg2);
			free(arg3);
			return buffer2;
		}
		else
			sprintf(buffer, "ERR\n");
	}

	else if(strcmp(com,"GSR")==0){
		n=sscanf(command, "%s %s %s %s\n",com, arg1, arg2, arg3);

		if(n==4){
			comSubscribe(buffer,arg1,arg2,arg3);
		}
		else
			sprintf(buffer, "ERR\n");
	}

	else if(strcmp(com,"GUR")==0){
		n=sscanf(command, "%s %s %s\n",com, arg1, arg2);

		if(n==3){
			comUnsubscribe(buffer,arg1,arg2);
		}
		else
			sprintf(buffer, "ERR\n");
	}
	else if(strcmp(com,"GLM")==0){
		n=sscanf(command, "%s %s\n",com, arg1);

		if(n==2){
			buffer2 = (char*) malloc(sizeof(char)*(7 + (currentGroups)*(24 + 4)));// TEST WITH 99 GROUPS
			comMyGroups(buffer2,arg1);

			free(com);
			free(arg1);
			free(arg2);
			free(arg3);
			return buffer2;
		}
		else
			sprintf(buffer, "ERR\n");
	}
	else if(strcmp(com,"ULS")==0){
		n=sscanf(command, "%s %s\n",com, arg1);

		if(n==2){
			comUList(buffer,arg1);//-----------------AJUSTAR BUFFER
		}
		else
			sprintf(buffer, "ERR\n");

		sendTCP(buffer, fd);
		free(com);
		free(arg1);
		free(arg2);
		free(arg3);
		return NULL;
	}
	else if(strcmp(com, "PST")==0){
		comPost(buffer, command);
		sendTCP(buffer, fd);

		free(com);
		free(arg1);
		free(arg2);
		free(arg3);
		return NULL;
	}
	else if(strcmp(com, "RTV") == 0){
		n = sscanf(command, "%s %s %s %s", com, arg1, arg2, arg3);

		if(n==4){
			//SendTCP included in comRetrieve
			comRetrieve(arg1, arg2, arg3, fd);
		}
		else{
			sprintf(buffer, "ERR\n");
			sendTCP(buffer, fd);
		}

		free(com);
		free(arg1);
		free(arg2);
		free(arg3);
		return NULL;
	}

    free(com);
	free(arg1);
    free(arg2);
	free(arg3);
	return buffer;
}



int main(int argc, char *argv[]){

	int fdUDP, fdTCP, newfd, maxfd;
	struct sigaction action;
	struct sockaddr_in addr;
	socklen_t addrlen;
	fd_set rfds;
	pid_t pid;

	memset(&action, 0, sizeof action);
	action.sa_handler = SIG_IGN;
	if(sigaction(SIGCHLD, &action, NULL) == -1)
		exit(1);

	if(strcmp(argv[argc-1],"-v") == 0){ //checking if -v flag was introduced
		verboseMode= true;
		printf("Running in verbose mode\n");
	}

	getNumberOfGroups();

	fdUDP = setSocketUDP();
	fdTCP = setSocketTCP();
	
	
	while(1){
		int n = 0; 

		FD_ZERO(&rfds);
		FD_SET(fdUDP,&rfds);
		FD_SET(fdTCP, &rfds);
		maxfd = max(fdTCP,fdUDP);

		n=select(maxfd+1,&rfds, (fd_set*) NULL,(fd_set*) NULL,(struct timeval *) NULL);
		if(n <= 0)/*error*/
			exit(1);

		
		if(FD_ISSET(fdTCP,&rfds)){ //TCP ready to be read
			addrlen=sizeof(addr);
			
			do newfd=accept(fdTCP,(struct sockaddr*)&addr,&addrlen);
			while(newfd==-1 && errno==EINTR);
			
			if(newfd==-1)
				exit(1);

			if((pid=fork())==-1)
				exit(1);
			else if(pid==0){
				close(fdTCP);

				int flags = fcntl(newfd, F_GETFL);
				fcntl(newfd, F_SETFL, flags | O_NONBLOCK);

				receiveTCP(newfd);

				close(newfd);
				exit(0);
			}

			do n=close(newfd);while(n==-1 && errno==EINTR);
			if(n==-1)
				exit(1);
		}

		if(FD_ISSET(fdUDP,&rfds)){ //UDP ready to be read
			n = receiveUDP(fdUDP);
			if(n == -1) 
				exit(1); //error occured

		}
	}
	close(fdUDP);
	close(fdTCP);
	
	exit(0);
}





