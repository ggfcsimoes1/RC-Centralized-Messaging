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
int DSport;

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
    DSport = DEFAULT_PORT + GROUP_NUMBER;
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
            DSport = stoi(args[i+1]);
            if(numArgs < 5)
                getDefaultIP();
        }
    }
}


int main(int argc, char *argv[]){

    DSIP = (char*) malloc(sizeof(char)); //ip adresses have 4 bytes


    verifyArguments(argc, argv);
    parseArgs(argc,argv);
    printf("%s %d\n", DSIP, DSport);

    free(DSIP); //meter numa func

    return 0;
}