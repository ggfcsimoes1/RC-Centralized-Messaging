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

bool isLoggedIn = false;
char* currentID;
char* currentPass;
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
char* clientSend(char* message){
    
    int fd,errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;
    
    fd=socket(AF_INET,SOCK_DGRAM,0);
    if(fd==-1)
        exit(1);
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;

    errcode=getaddrinfo(DSIP,DSport,&hints,&res);
    if(errcode!=0)
        exit(1);
    
    n=sendto(fd,message,strlen(message),0,res->ai_addr,res->ai_addrlen);
    if(n==-1)
        exit(1);
    addrlen=sizeof(addr);

    free(message);
    message = (char*) malloc(sizeof(char)*SIZE_STRING);
    
    n=recvfrom(fd,message,SIZE_STRING,0,(struct sockaddr*)&addr,&addrlen); //nao está a receber bem do server
    
    if(n==-1)
        exit(1);

    freeaddrinfo(res);
    close(fd);
    return message;
}

void commandRegister(char* message){
    char* response = clientSend(message);

    printf("%s\n", response);
    if(strcmp("ERR\n",response)==0)
        fprintf(stderr,"Registration error\n");

    else if(strcmp("RRG OK\n",response)==0)  
        printf("Accepted Registration!\n"); 

    else if(strcmp("RRG NOK\n",response)==0)      
        printf("Not Accepted Registration!\n"); 

    else if(strcmp("RRG DUP\n",response)==0)      
        printf("Duplicate registration!\n"); 
    
    else   
        printf("Unexpected error\n");
}

void commandUnregister(char* message){

    //fazer logout

    char* response = clientSend(message);
    if(strcmp("ERR\n",response)==0)
        fprintf(stderr,"Un-Registration error\n");

    else if(strcmp("RUN OK\n",response)==0)  
        printf("Accepted Un-Registration!\n"); 

    else if(strcmp("RUN NOK\n",response)==0)      
        printf("Not Accepted Un-Registration!\n"); 
    else   
        printf("Unexpected error\n");
}


void commandLogin(char* message, char* UID, char* pass){
    char* response = clientSend(message);
    if(strcmp("ERR\n",response)==0){
        fprintf(stderr,"Login error\n");
    }

    else if(strcmp("RLO OK\n",response)==0){  
        printf("Accepted Login!\n"); 
        currentID = UID;
        currentPass = pass;
        isLoggedIn = true;
    }

    else if(strcmp("RLO NOK\n",response)==0){    
        printf("Not Accepted Login!\n");
    }
    else{   
        printf("Unexpected error\n");
    }
}

void commandLogout(char* message, char* UID, char* pass){
    char* response = clientSend(message);
    if(strcmp("ERR\n",response)==0){
        fprintf(stderr,"Logout error\n");
    }

    else if(strcmp("ROU OK\n",response)==0){  
        printf("Accepted Logout!\n"); 
        currentID = NULL;
        currentPass = NULL;
        isLoggedIn = false;
    }

    else if(strcmp("ROU NOK\n",response)==0){    
        printf("Not Accepted Logout!\n");
    }
    else{ 
        printf("%s\n",response) ;
        printf("Unexpected error\n");
    }
}

void commandShowUID(){
    if(isLoggedIn)
        printf("User ID: %s \n", currentID);
    else 
        printf("Not logged in!\n");   
}

void processCommands(){
    
    char* buffer=(char*) malloc(sizeof(char)*SIZE_STRING);
    char *com,*pass,*UID;
    com=(char*) malloc(sizeof(char)*SIZE_STRING);
    pass=(char*) malloc(sizeof(char)*8);
    UID=(char*) malloc(sizeof(char)*5);// HARDCODED
    int n;
    
    
    while(fgets(buffer,SIZE_STRING,stdin)){
        n=sscanf(buffer,"%s %s %s",com,UID,pass); 
        strcpy(buffer, "");


        if(n<1)
            continue;

        if(strcmp(com,"reg")==0){
            sprintf(buffer, "REG %s %s\n", UID, pass);
            commandRegister(buffer);
        }
            
        else if(strcmp(com,"unregister")==0 || strcmp(com,"unr")==0){
            sprintf(buffer, "UNR %s %s\n", UID, pass);
            commandUnregister(buffer);
        }
            
        else if(strcmp(com,"login")==0){
            sprintf(buffer, "LOG %s %s\n", UID, pass);
            //O QUE É SUPOSTO RESPONDER OH BURRO
            if(!isLoggedIn)
                commandLogin(buffer, UID, pass);
            else
                printf("FUCK YOU BITCH JA DERAM LOGIN\n");
        }

        else if(strcmp(com,"logout")==0){
            sprintf(buffer, "OUT %s %s\n", UID, pass);
            if(isLoggedIn)
                commandLogout(buffer, UID, pass);
            else
                printf("ATAO PRIMO NINGUEM TA LOGADO QUERES DAR LOGOUT?\n");
        }

        else if(strcmp(com,"showuid")==0 || strcmp(com,"su")==0){
            commandShowUID();
        }
        else if(strcmp(com,"exit")==0)
            //fechar TCP
            exit(0);
                
    }
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