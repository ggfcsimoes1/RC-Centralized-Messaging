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
	if((errcode=getnameinfo((struct sockaddr*)&addr,addrlen,host,sizeof host,service,sizeof service,0))!=0)
		fprintf(stderr,"error:getnameinfo: %s\n",gai_strerror(errcode));
	else if (verboseMode){ //if the server is running with -v
		printf("sent by [%s:%s]\n",host,service);
		//dar print no process commands do GID e UID, e a descricao do comando
	}
	buffer2 = processCommands(buffer);
	
	n=sendto(fd,buffer2,strlen(buffer2),0, (struct sockaddr*) &addr, addrlen);
	if(n==-1)
		return -1;

	return 0;
}

void receiveTCP(int fd){
    int errcode, i = 1;
    ssize_t n, toWrite;
    struct addrinfo hints, *res;
    char buffer[11];
    char * buffer2, *message, *ptr;

	FILE* fp2;

    message = NULL;
    memset(buffer, 0, sizeof(buffer));

	fp2 = fopen("gustavo.jpg", "wb");

    while(1){

		n=read(fd,buffer, 10);

		if(n == -1 && errno == EWOULDBLOCK){
        	break;
    	}
		else if(n == -1){
			exit(1);
		}

		printf("%d\n", i);
		fwrite(buffer,1,n,fp2);

        message =(char*) realloc(message, sizeof(char) * ((i * 10) + 1));

        if(i == 1){
            memset(message, 0, sizeof(message));
        }

        strcat(message, buffer);
        
        i++;

        memset(buffer, 0, sizeof(buffer));
        
    }
 
	fclose(fp2);

	printf("-------message: %s\n", message);

    

    //buffer2 = processCommands(message);

    ptr = buffer2;
    toWrite = strlen(buffer2);

    while(toWrite > 0){
        n=write(fd, ptr, toWrite);

        if(n<=0)
            exit(1);

        toWrite-= n;
        ptr+= n;
    }

    free(buffer2);
    free(message);
}

/*void receiveTCP(int fd){
    int errcode, i = 0;
    ssize_t n, toWrite;
    struct addrinfo hints, *res;
    char c[1];
	char * buffer2, *message, *ptr;

	message = NULL;

    while(1){
        if((n=read(fd, c, 1)) == -1){
            exit(1);
        }
        else if(n == 0){
            continue;
        }

        i++;

        message =(char*) realloc(message, sizeof(char) * (i + 1));

        if(i == 1){
            memset(message, 0, sizeof(message));
        }
        
        strcat(message, c);

        if(strcmp(c, "\n") == 0){
            break;
        }
    }

	buffer2 = processCommands(message);

	//printf("message: %s\n", buffer2);

	ptr = buffer2;
    toWrite = strlen(buffer2);

    while(toWrite > 0){
        n=write(fd, ptr, toWrite);

        if(n<=0)
            exit(1);

        toWrite-= n;
        ptr+= n;
    }

	free(buffer2);
	free(message);
}*/

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

bool verifyArguments(char* buffer,char* password, int UID){
	
    if ((UID/100000) != 0 || strlen(password)!=8){ 
        sprintf(buffer, "RRG NOK\n");

        return false;
    }
    else
        return true;
}

bool verifyPassword(char*fileDirectory, char* password,int UID,char* buffer){
	FILE *fp;
	bool verified=false;
	char* pass=(char*)malloc(sizeof(char)*9);

	sprintf(fileDirectory,"USERS/%d/%d_pass.txt",UID ,UID);

	if((fp=fopen(fileDirectory,"r"))==NULL ){
        if(errno!=ENOENT)
            sprintf(buffer, "ERR\n");
        free(pass);
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
	
	if(!verifyArguments(buffer,pass,UID)){
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
    DIR *d;
    struct dirent *dir;
    int i=0;
    char GIDname[30];
    char *fileDirectory = (char*) malloc(sizeof(char) * SIZE_STRING);//HARDCODEDs

    sprintf(buffer, "RUN NOK\n");

    if(verifyPassword(fileDirectory,pass,UID,buffer)){
        if(remove(fileDirectory)==0){
            sprintf(fileDirectory,"USERS/%d", UID);

            if(logout(UID) == false || rmdir(fileDirectory) != 0){
                sprintf(buffer, "ERR\n");
                free(fileDirectory);
                return;
            }
            d = opendir("GROUPS");
            if (d)    {
                while ((dir = readdir(d)) != NULL)    {
                    if(dir->d_name[0]=='.')
                        continue;
                    if(strlen(dir->d_name)>2)
                        continue;

                    sprintf(GIDname,"GROUPS/%s/%05d.txt",dir->d_name,UID);

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
			if(fp!=NULL)	{
				fscanf(fp,"%24s",GIDname);
				fclose(fp);
			}

			sprintf(buffer, "%s %s %s 0000", buffer, dir->d_name, GIDname);
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
	char* name = (char*) malloc(sizeof(char)*SIZE_STRING);

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

		sprintf(fileDirectory, "GROUPS/%02d/%05d.txt", currentGroups, UID);
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
		sprintf(fileDirectory, "GROUPS/%s/%s_name.txt", GID, GID);
		
		if((fp = fopen(fileDirectory, "r")) == NULL){
			sprintf(buffer, "ERR\n");
			free(fileDirectory);
			return;
		}
		else{
			name = (char*) malloc(sizeof(char)*SIZE_STRING);
			fscanf(fp,"%s\n",name);

			if(strcmp(name,GNAME)!=0){
				sprintf(buffer, "RGS E_GNAME\n");
				fclose(fp);
				free(name);
				free(fileDirectory);
				return;
			}
			
			fclose(fp);
			free(name);
			
		}

		sprintf(fileDirectory, "GROUPS/%s/%05d.txt", GID, UID);
		if((fp = fopen(fileDirectory, "w+")) == NULL){
			sprintf(buffer, "ERR\n");
			free(fileDirectory);
			return;
		}
		else{
			sprintf(buffer,"RGS OK\n");
			fclose(fp);
		}

	} 
	else if (currentGroups==99) //when there's 99 groups registered already (directory is full)
		sprintf(buffer, "RGS E_FULL");
}

void comUnsubscribe(char* buffer, int UID, char* GID){

	char* fileDirectory = (char*) malloc(sizeof(char)*SIZE_STRING);
	sprintf(fileDirectory, "GROUPS/%s/%05d.txt", GID, UID);

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

void comMyGroups(char* buffer,int uid){
	DIR *d;
	struct dirent *dir;
	int i=0,numGroups=0;
	FILE *fp;
	char GIDname[30];
	char* aux=(char*)malloc(sizeof(char)*10000);// Alterar valor

	memset(aux, 0, sizeof(aux));
	sprintf(aux, "USERS/%d/%d_pass.txt", uid, uid);

	if((fp = fopen(aux, "r")) == NULL){
		sprintf(buffer, "RGM E_USR\n");
		free(aux);
		return;
	}

	fclose(fp);

	memset(aux, 0, sizeof(aux));
	memset(buffer, 0, sizeof(buffer));

	d = opendir("GROUPS");

	if (d)	{
		while ((dir = readdir(d)) != NULL)	{
			if(dir->d_name[0]=='.')
				continue;
			if(strlen(dir->d_name)>2)
				continue;

			sprintf(GIDname,"GROUPS/%s/%05d.txt",dir->d_name,uid);
			fp=fopen(GIDname,"r");
			if(fp != NULL)	{
				fclose(fp);
				numGroups++;
				sprintf(GIDname,"GROUPS/%s/%s_name.txt",dir->d_name,dir->d_name);
				fp=fopen(GIDname,"r");
				if(fp)	{
					fscanf(fp,"%24s",GIDname);
					fclose(fp);
				}

				sprintf(aux, "%s %s %s 0000", aux, dir->d_name, GIDname);	
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
	free(aux);
}

void comUList(char* buffer,int gid){
	DIR *d;
	struct dirent *dir;
	FILE *fp;
	char GIDname[30], user[6];

	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "GROUPS/%02d", gid);
	d = opendir(buffer);

	memset(buffer, 0, sizeof(buffer));

	if (d)	{
		sprintf(GIDname,"GROUPS/%02d/%02d_name.txt", gid, gid);
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

void comPost(char* buffer, char* command){
	char aux[3], uid[5], gid[2], fileName[24], tsize[3], fsize[10];
	int n;
	long f;
	char *fileDir, *text;
	FILE *fp2;

	memset(tsize, 0, sizeof(tsize));
	memset(fsize, 0, sizeof(fsize));

	n=sscanf(command, "%s %s %s %s", aux, uid, gid, tsize);

	if(n < 4){
		sprintf(buffer, "ERR\n");// --------------Verificar depois
		return;
	}
	
	n = atoi(tsize);

	text= (char*) malloc(sizeof(char)*(n + 1));

	memset(text, 0, sizeof(text));
	memset(fileName, 0, sizeof(fileName));

	command += (14 + strlen(tsize));
	strncpy(text, command, n);

	command += (n + 1);

	n=sscanf(command, "%s %s",fileName, fsize);{}

	if(strcmp(fileName, "") != 0){
		f = atoi(fsize);
		command += strlen(fileName) + strlen(fsize) + 2;

        fp2 = fopen("output.jpg", "wb"); 
		fwrite(command,1,f,fp2);

		fclose(fp2);
	}

	printf("text: %s\nfileName: %s\n", text, fileName);

}

char* processCommands(char* command){
	char* groupNames;
    char* com = (char*) malloc(sizeof(char)*SIZE_STRING);
    char* arg2=(char*) malloc(sizeof(char)*SIZE_STRING);
	char* arg3=(char*) malloc(sizeof(char)*SIZE_GROUP_NAMES);
    char* buffer=(char*) malloc(sizeof(char)*SIZE_STRING);
    int n,arg1;



    n=sscanf(command,"%s",com); 

    if(n<1)
		sprintf(buffer, "ERR\n");
        
	if(verboseMode) printf("%s",command); //printing the command if the server is running with -v

    if(strcmp(com,"REG")==0 ){
		
		n=sscanf(command, "%s %d %s\n",com, &arg1, arg2);

		if(n==3){
			comRegister(buffer,arg1,arg2);
		}
		else
			sprintf(buffer, "ERR\n");
    }
            
    else if(strcmp(com,"UNR")==0){
		n=sscanf(command, "%s %d %s\n",com, &arg1, arg2);

		if(n==3)
        	comUnregister(buffer,arg1,arg2);
		else
			sprintf(buffer, "ERR\n");
	}
        
    else if(strcmp(com,"LOG")==0){
		n=sscanf(command, "%s %d %s\n",com, &arg1, arg2);

		if(n==3)
        	comLogin(buffer,arg1,arg2);
		else
			sprintf(buffer, "ERR\n");
	}

    else if(strcmp(com,"OUT")==0){
		n=sscanf(command, "%s %d %s\n",com, &arg1, arg2);

		if(n==3)
        	comLogout(buffer,arg1,arg2);
		else
			sprintf(buffer, "ERR\n");
	}

	else if(strcmp(com,"GLS")==0){
		
		if(n==1){
			groupNames = (char*) malloc(sizeof(char)*(7 + (currentGroups)*(24 + 4)));// TEST WITH 99 GROUPS
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
		n=sscanf(command, "%s %d %s %s\n",com, &arg1, arg2, arg3);

		if(n==4){
			comSubscribe(buffer,arg1,arg2,arg3);
		}
		else
			sprintf(buffer, "ERR\n");
	}

	else if(strcmp(com,"GUR")==0){
		n=sscanf(command, "%s %d %s\n",com, &arg1, arg2);

		if(n==3){
			comUnsubscribe(buffer,arg1,arg2);
		}
		else
			sprintf(buffer, "ERR\n");
	}
	else if(strcmp(com,"GLM")==0){
		n=sscanf(command, "%s %d\n",com, &arg1);

		if(n==2){
			groupNames = (char*) malloc(sizeof(char)*(7 + (currentGroups)*(24 + 4)));// TEST WITH 99 GROUPS
			comMyGroups(groupNames,arg1);
			free(com);
			free(arg2);
			free(arg3);
			return groupNames;
		}
		else
			sprintf(buffer, "ERR\n");
	}
	else if(strcmp(com,"ULS")==0){
		n=sscanf(command, "%s %d\n",com, &arg1);

		if(n==2){
			comUList(buffer,arg1);//-----------------AJUSTAR BUFFER
		}
		else
			sprintf(buffer, "ERR\n");
	}
	else if(strcmp(com, "PST")==0){

		comPost(buffer, command);

		/*FILE *fp2;
        fp2 = fopen("output.jpg", "wb"); 
		fwrite(oi6,sizeof(char),b,fp2);
        
		printf("com: %s\n", command);
		fclose(fp2);*/
		
	}

    free(com);
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





