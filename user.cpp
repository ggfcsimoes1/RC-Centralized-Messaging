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
char* currentUID;
char* currentPass;
int currentGID=0;
char* DSIP;
char* DSport;


int TimerON(int sd){
    struct timeval tmout;
    memset((char *)&tmout,0,sizeof(tmout)); /* clear time structure */
    tmout.tv_sec=15; /* Wait for 15 sec for a reply from server. */
    return(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tmout,sizeof(struct timeval)));
}

int TimerOFF(int sd){
    struct timeval tmout;
    memset((char *)&tmout,0,sizeof(tmout)); /* clear time structure */
    return(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tmout,sizeof(struct timeval)));
}



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
char* clientSend(char* message, int sizeString){
    
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

    message = (char*) malloc(sizeof(char)* sizeString);

    TimerON(fd);
    n=recvfrom(fd,message,sizeString,0,(struct sockaddr*)&addr,&addrlen); //recebe mensagens do servidor

    if(errno == EAGAIN){  
        printf("Timeout occured!\n");
    }
    else if(n==-1)
        exit(1);

    TimerOFF(fd);
    freeaddrinfo(res);
    close(fd);
    return message;
}

void commandRegister(char* message){
    char* response = clientSend(message,SIZE_STRING);

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

    char* response = clientSend(message,SIZE_STRING);
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
    char* response = clientSend(message,SIZE_STRING);
    if(strcmp("ERR\n",response)==0){
        fprintf(stderr,"Login error\n");
    }

    else if(strcmp("RLO OK\n",response)==0){  
        printf("Accepted Login!\n"); 
        strcpy(currentUID, UID);
        strcpy(currentPass, pass);
        isLoggedIn = true;
    }

    else if(strcmp("RLO NOK\n",response)==0){    
        printf("Not Accepted Login!\n");
    }
    else{   
        printf("Unexpected error\n");
    }
}

void commandLogout(char* message){
    char* response = clientSend(message,SIZE_STRING);
    if(strcmp("ERR\n",response)==0){
        fprintf(stderr,"Logout error\n");
    }

    else if(strcmp("ROU OK\n",response)==0){  
        printf("Accepted Logout!\n"); 
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
        printf("User ID: %s \n", currentUID);
    else 
        printf("Not logged in!\n");   
}

void commandGroups(char* message){
    

    char* response = clientSend(message,10000);
    if(strcmp("ERR\n",response)==0){
        fprintf(stderr,"Logout error\n");
    }
    printf("%s\n",response);

}

void commandSubscribe(char* message){
    char* GID = (char*) malloc(sizeof(char)* 3);
    
    char* response = clientSend(message,SIZE_STRING);
    
    if(strcmp("ERR\n",response)==0){
        fprintf(stderr,"Subscribe error\n");
    }
    else if(strcmp("RGS OK\n", response) == 0){
        printf("Accepted Subscription!\n");
    }
    else if(sscanf(response, "RGS NEW %s", GID) == 1){
        printf("Accepted Subscription!\nGID: %s\n", GID);
    }
    else if(strcmp("RGS E_USR\n",response) == 0){    
        printf("Sub: Invalid user!\n");
    }
    else if(strcmp("RGS E_GRP\n",response) == 0){    
        printf("Sub: Invalid group ID!\n");
    }
    else if(strcmp("RGS E_GNAME\n",response) == 0){    
        printf("Sub: Invalid group name!\n");
    }
    else if(strcmp("RGS E_FULL\n",response) == 0){    
        printf("Sub: Maximum groups reached!\n");
    }
    else if(strcmp("RGS NOK\n",response) == 0){    
        printf("Not Accepted Subscription!\n");
    }
    else{   
        printf("Unexpected error\n");
    }
}

void commandUnsubscribe(char* message){
    char* GID = (char*) malloc(sizeof(char)* 3);
    char* response = clientSend(message,SIZE_STRING);

    if(strcmp("ERR\n",response)==0)
        fprintf(stderr,"Unsubscribe error\n");

    else if(strcmp("RGU OK\n", response) == 0)
        printf("Accepted unsubscription!\n");

    else if(strcmp("RGU E_USR\n", response) == 0)
        printf("Unsub: Invalid user!\n");

    else if(strcmp("RGU E_GRP\n", response) == 0)
        printf("Unsub: Invalid group ID!\n");

    else if(strcmp("RGU NOK\n", response) == 0)
        printf("Not Accepted unsubscription!\n");

    else
        printf("Unexpected error\n");
    
}

void commandSelect(char* GID){
    if (atoi(GID) > 0 && atoi(GID) < 100){
        currentGID = atoi(GID);
        printf("Selected Group ID: %d \n", currentGID);
    }
    else 
        printf("Invalid Group ID!\n");
}

void commandShowGID(){
    if (currentGID != 0){;
        printf("Current Group ID: %d \n", currentGID);
    }
    else 
        printf("No group selected!\n");
}

void commandMyGroups(char* message){
   if (isLoggedIn){ 
        
        char* response = clientSend(message,10000);
        printf("%s\n",response);
    }
    else 
        printf("No login\n");
}

void processCommands(){
    
    char* buffer=(char*) malloc(sizeof(char)*SIZE_STRING);
    char* com = (char*) malloc(sizeof(char)*SIZE_STRING);
    char* arg2=(char*) malloc(sizeof(char)*8);
    //char* arg3=(char*) malloc(sizeof(char)*SIZE_GROUP_NAMES);
    char* arg1=(char*) malloc(sizeof(char)*5);// HARDCODED
    int n;

    //VERIFICAR ARGUMENTOS

    while(fgets(buffer,SIZE_STRING,stdin)){
        n=sscanf(buffer,"%s %s %s",com,arg1,arg2); 
        strcpy(buffer, "");


        if(n<1)
            continue;

        if(strcmp(com,"reg")==0){
            if(n==3){
                sprintf(buffer, "REG %s %s\n", arg1, arg2);
                commandRegister(buffer);
            }
            else
                printf("zaga zaga\n");

           

        }
            
        else if(strcmp(com,"unregister")==0 || strcmp(com,"unr")==0){
            
                sprintf(buffer, "UNR %s %s\n", arg1, arg2);
                commandUnregister(buffer);
            
            
        }
            
        else if(strcmp(com,"login")==0){
            
                sprintf(buffer, "LOG %s %s\n", arg1, arg2);
            
                if(!isLoggedIn)
                    commandLogin(buffer, arg1, arg2);
                else
                    printf("Already logged in\n");
            
            

            
        }

        else if(strcmp(com,"logout")==0){
            
                sprintf(buffer, "OUT %s %s\n", arg1, arg2);
                if(isLoggedIn)
                    commandLogout(buffer);
                else
                    printf("No login\n");
                        

           
        }

        else if(strcmp(com,"showuid")==0 || strcmp(com,"su")==0){
            
                commandShowUID();
            
        }

        else if(strcmp(com,"groups")==0 || strcmp(com,"gl")==0){
            
                sprintf(buffer, "GLS\n");
                commandGroups(buffer);
                       
            
        }

        else if(strcmp(com,"subscribe")==0 || strcmp(com,"s")==0){  
            
                sprintf(buffer, "GSR %s %s %s\n", currentUID, arg1, arg2);
                commandSubscribe(buffer);
            
           
            
        }    

        else if(strcmp(com,"unsubscribe")==0 || strcmp(com,"u")==0){  
            
                sprintf(buffer, "GUR %s %s\n", currentUID, arg1);
                commandUnsubscribe(buffer);
              
            
        }   

        else if(n == 2 && (strcmp(com,"select")==0 || strcmp(com,"sag")==0)){  //no of arguments have to be verified locally 
            commandSelect(arg1);
        }  

        else if(n == 1 && (strcmp(com,"showgid")==0 || strcmp(com,"sg")==0)){   //no of arguments have to be verified locally 
            commandShowGID();
        }  
        else if(n == 1 && (strcmp(com,"my_groups")==0 || strcmp(com,"mgl")==0)){   //no of arguments have to be verified locally 
            sprintf(buffer, "GLM %s\n", currentUID);
            commandMyGroups(buffer);
        }  
        else if(strcmp(com,"exit")==0)
            //fechar TCP
            exit(0);
        else
            fprintf(stderr, "Invalid command!\n");
        
    }
}

int main(int argc, char *argv[]){

    DSIP = (char*) malloc(sizeof(char)); //ip adresses have 4 bytes
    DSport = (char*) malloc(sizeof(char));
    currentUID = (char*) malloc(sizeof(char) * 5);
    currentPass = (char*) malloc(sizeof(char) * 8);

    verifyArguments(argc, argv);
    parseArgs(argc,argv);
    
    processCommands();
    
    free(DSIP); //meter numa func

    return 0;
}