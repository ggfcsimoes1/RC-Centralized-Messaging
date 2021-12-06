#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "user.hpp"


using namespace std;


char* DSIP;
char* DSport;

void verifyArguments(int numArgs, char *args[]){
    if (numArgs != 1 && numArgs != 3 && numArgs != 5){ //-n e -p sempre como argumento?
        fprintf(stderr,"Invalid number of Arguments!");
        exit(EXIT_FAILURE);
    }
    return; //perguntar se precisamos de verificar mais
}

void getDefaultIP(){

    struct addrinfo hints,*res,*p;
    int errcode;
    char buffer[INET_ADDRSTRLEN];
    struct in_addr *addr;
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;//IPv4
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_CANONNAME;

    char buffer1[128];
    if(gethostname(buffer1,128)==-1){
        fprintf(stderr,"error: %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    if((errcode=getaddrinfo(buffer1,NULL,&hints,&res))!=0){
        fprintf(stderr,"error: getaddrinfo: %s\n",gai_strerror(errcode));
        exit(EXIT_FAILURE);
    }
    else{
        for(p=res;p!=NULL;p=p->ai_next){
            addr=&((struct sockaddr_in *)p->ai_addr)->sin_addr;
            strcpy(DSIP, inet_ntop(p->ai_family,addr,buffer,sizeof buffer));  //(long unsigned int)ntohl(addr->s_addr));;
        }

        
        //DSIP = string(DSIP);
        freeaddrinfo(res);
    }
}

void getDefaultPort(){
    int port= DEFAULT_PORT + GROUP_NUMBER;
    sprintf(DSport, "%d", port);
    
}


void parseArgs(int numArgs,char *args[]){
    if (numArgs == 1){
        getDefaultIP();
        getDefaultPort();
    }
    for(int i=1;i<numArgs;i+=2){
        if(strcmp(args[i],"-n") == 0){
            strcpy(DSIP, args[i+1]);
            if(numArgs < 5)
                getDefaultPort();
        }
        else if(strcmp(args[i],"-p") == 0){
            strcpy(DSport ,args[i+1]);
            if(numArgs < 5)
                getDefaultIP();
        }
    }
}
void client(char* message){
    int fd,errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;
    char buffer[SIZE_STRING];

    fd=socket(AF_INET,SOCK_DGRAM,0);
    if(fd==-1)
        exit(1);
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;

    errcode=getaddrinfo(DSIP,DSport,&hints,&res);
    if(errcode!=0)
        exit(1);
    
    n=sendto(fd,message,sizeof(message),0,res->ai_addr,res->ai_addrlen);
    if(n==-1)
        exit(1);
    addrlen=sizeof(addr);
    
    n=recvfrom(fd,buffer,SIZE_STRING,0,(struct sockaddr*)&addr,&addrlen);
    if(n==-1)
        exit(1);

    freeaddrinfo(res);
    close(fd);
}

void processCommands(){
    char *com,*pass;
    com=(char*) malloc(sizeof(char)*SIZE_STRING);
    pass=(char*) malloc(sizeof(char)*SIZE_STRING);

    int n,UID;
    while(true){

        n=sscanf("%s %d %s\n",com,UID,pass);
        
        if(n<1)
           continue;
        
        printf("%s %d %s %d\n",com,UID,pass,n);
        
        
        
    }
    free(com);
    free(pass);


}

int main(int argc, char *argv[]){

    DSIP = (char*) malloc(sizeof(char)); //ip adresses have 4 bytes
    DSport = (char*) malloc(sizeof(char));

    verifyArguments(argc, argv);
    parseArgs(argc,argv);
    processCommands();
    
    free(DSIP); //meter numa func

    return 0;
}