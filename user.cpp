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
char* clientSendUDP(char* message, int sizeString){
    
    int fd,errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;
    char* response;
    
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

    response = (char*) malloc(sizeof(char)* sizeString);

    memset(response, 0, sizeof(response));

    TimerON(fd);
    n=recvfrom(fd,response,sizeString,0,(struct sockaddr*)&addr,&addrlen); //recebe mensagens do servidor

    //Remover lixo no final da resposta
    sscanf(response, "%[^\n]", response);
    strcat(response, "\n");

    if(errno == EAGAIN){  
        printf("Timeout occured!\n");
    }
    else if(n==-1)
        exit(1);

    TimerOFF(fd);
    freeaddrinfo(res);
    close(fd);
    return response;
}

/*char* clientSendTCP(char* message){
    int fd, errcode;
    ssize_t n, toWrite;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char c[1];
    char *response, *ptr;
    int i = 0;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd==-1) exit(1);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    errcode=getaddrinfo(DSIP, DSport, &hints, &res);
    if(errcode!=0) exit(1);

    n = connect(fd, res-> ai_addr, res-> ai_addrlen);
    if(n==-1) exit(1);

    ptr = message;
    toWrite = strlen(message);
    
    while(toWrite > 0){
        n=write(fd, ptr, toWrite);

        if(n<=0)
            exit(1);

        toWrite-= n;
        ptr+= n;
    }
    
    response = NULL;

    while(1){
        if((n=read(fd, c, 1)) == -1){
            exit(1);
        }
        else if(n == 0){
            continue;
        }

        i++;

        response =(char*) realloc(response, sizeof(char) * (i + 1));

        if(i == 1){
            memset(response, 0, sizeof(response));
        }
        
        strcat(response, c);


        if(strcmp(c, "\n") == 0){
            break;
        }
    }

    freeaddrinfo(res);
    close(fd);
    
    return response;
}*/

void fileSendTCP(char* filename, int fd){
    FILE* fp = fopen(filename, "rb");
    char buffer[11];

    bzero(buffer, sizeof(buffer)); //just in case

    while(!feof(fp)) {
        fread(buffer, 1, sizeof(buffer), fp);
        write(fd, buffer, sizeof(buffer));
        bzero(buffer, sizeof(buffer));
    }
}

char* clientSendTCP(char* message, char* fileName){
    int fd, errcode;
    ssize_t n = 1;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char *response, *ptr;
    int i = 1, nread = 0;
    int messageSize= strlen(message);

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd==-1) exit(1);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    errcode=getaddrinfo(DSIP, DSport, &hints, &res);
    if(errcode!=0) exit(1);

    n = connect(fd, res-> ai_addr, res-> ai_addrlen);
    if(n==-1) exit(1);

    ptr = message;
    

    while(messageSize > 0){
        n=write(fd, ptr, messageSize);

        printf("%ld\n", n);
        if(n<=0)
            exit(1);

        messageSize-= n;
        ptr+= n;
    }

    
    if(fileName != NULL)
        fileSendTCP(fileName, fd);
    
    response = NULL;

    while(n > 0){

        response =(char*) realloc(response, sizeof(char) * ((i * 10) + 1));

        if(i == 1){
            memset(response, 0, sizeof(response));
        }

        n=read(fd, response + nread, 10);

        nread += n;
        i++;
       
    }

    if(n == -1){
        exit(1);
    }

    freeaddrinfo(res);
    close(fd);
    
    return response;
}

char* verifyFile(char fileName[], long* fsize){
    FILE *fp;
     
    int nameSize = strlen(fileName);
    char* data;

    //Verify file name
    for(int i = 0; i < nameSize; i++){
        if(!isalpha(fileName[i]) && !isdigit(fileName[i]) && fileName[i] != '_' && fileName[i] != '-' && fileName[i] != '.'){
            printf("Invalid file name\n");
            return NULL;
        }
    }

    //read file
    if((fp = fopen(fileName, "r")) != NULL){
        
        fseek(fp, 0, SEEK_END);
        *fsize = ftell(fp); 
        fseek(fp, 0, SEEK_SET); 

        data = (char*) malloc(*fsize);
        
        if(fread(data,1,*fsize,fp) != *fsize){ //doesn't distinguish between EOF and read errors
            printf("Reading error\n");
            return NULL;
        }

        /*FILE* fp2;

        fp2 = fopen("output.jpg", "wb"); 
		fwrite(data,1,*fsize,fp2);

		fclose(fp2);*/
        
        return data;

    }
    else if (errno == ENOENT){
        printf("No file named %s\n", fileName);
        return NULL;
    }
    else{
        printf("Error opening file\n");
        return NULL;
    }
}

void commandRegister(char* message){
    char* response = clientSendUDP(message,SIZE_STRING);

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
    
    free(response);
}

void commandUnregister(char* message, char* arg1){
    char* response = clientSendUDP(message,SIZE_STRING);

    if(strcmp("ERR\n",response)==0)
        fprintf(stderr,"Un-Registration error\n");

    else if(strcmp("RUN OK\n",response)==0) { 
        printf("Accepted Un-Registration!\n");


        if(isLoggedIn && strcmp(currentUID,arg1)==0){
            isLoggedIn=false;
        }

    }
    else if(strcmp("RUN NOK\n",response)==0)
        printf("Not Accepted Un-Registration!\n"); 
    else
        printf("Unexpected error\n");
    
    free(response);

}

void commandLogin(char* message, char* UID, char* pass){
    char* response = clientSendUDP(message,SIZE_STRING);
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
    free(response);
}

void commandLogout(char* message){
    char* response = clientSendUDP(message,SIZE_STRING);
    if(strcmp("ERR\n",response)==0){
        fprintf(stderr,"Logout error\n");
    }

    else if(strcmp("ROU OK\n",response)==0){  
        printf("Accepted Logout!\n");
        strcpy(currentUID, "");
        strcpy(currentPass, ""); 
        isLoggedIn = false;
    }

    else if(strcmp("ROU NOK\n",response)==0){    
        printf("Not Accepted Logout!\n");
    }
    else{ 
        printf("Unexpected error\n");
    }
    free(response);
}

void commandShowUID(){
    if(isLoggedIn)
        printf("User ID: %s \n", currentUID);
    else 
        printf("Not logged in!\n");   
}

void commandGroups(char* message){
    char* response = clientSendUDP(message,10000);
    char com[4], gname[24];
    int ng, n, gid, mid;

    if(strcmp("ERR\n",response)==0){
        fprintf(stderr,"Group error\n");
        free(response);
        return;
    }
    
    n=sscanf(response,"%s %d %[^\n]",com, &ng, response); 
    if(n < 3){
        printf("Unexpected error\n");
        free(response);
        return;
    }
    if(strcmp(com, "RGL")==0 && ng == 0){
        printf("No Groups Available!\n");
    }
    else if(strcmp(com, "RGL")==0 && ng > 0 && ng <= 99){
        printf("Available Groups:\n");
        while(ng != 0){
            sscanf(response,"%d %s %d %[^\n]",&gid, gname, &mid,response);
            printf("> %02d: %s [msg: %04d]\n", gid, gname, mid);
            ng--;
        }
    }
    else{
        printf("Unexpected error\n");
    }
    free(response);
}

void commandSubscribe(char* message){
    char* GID = (char*) malloc(sizeof(char)* 3);
    
    char* response = clientSendUDP(message,SIZE_STRING);
    
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
    free(response);
}

void commandUnsubscribe(char* message){
    char* GID = (char*) malloc(sizeof(char)* 3);
    char* response = clientSendUDP(message,SIZE_STRING);

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
    
    free(response);
    
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
    char* response;
    char com[4], gname[24];
    int ng, n, gid, mid;

    response = clientSendUDP(message,10000);
        
    if(strcmp("ERR\n",response)==0){
        fprintf(stderr,"My_Groups error\n");
        free(response);
        return;
    }
    else if(strcmp("RGM E_USR\n", response)== 0){
        printf("Invalid User!\n");
        free(response);
        return;
    }
    
    n=sscanf(response,"%s %d %[^\n]",com, &ng, response); 
    if(n < 2){
        printf("Unexpected error\n");
        free(response);
        return;
    }

    if(strcmp(com, "RGM")==0 && ng == 0){
        printf("No Groups Subscribed To!\n");
    }
    else if(strcmp(com, "RGM")==0 && ng > 0 && ng <= 99){
        printf("My Groups:\n");
        
        while(ng != 0){
            sscanf(response,"%d %s %d %[^\n]",&gid, gname, &mid,response);
            printf("> %02d: %s [msg: %04d]\n", gid, gname, mid);
            ng--;
        }
    }
    else{
        printf("Unexpected error\n");
    }

    free(response);
}

void commandUList(char* message){
    char* response = clientSendTCP(message, NULL);
    char* users = (char *) malloc(sizeof(char) * strlen(response));
    char com[4], gname[24], status[4];
    int n, uid;


    memset(users, 0, sizeof(users));
    if(strcmp("ERR\n",response)==0){
        fprintf(stderr,"UList error\n");
        free(response);
        return;
    }

    n=sscanf(response,"%s %s %s %[^\n]",com, status, gname, users);

    if(n < 2){
        printf("Unexpected error\n");
        free(response);
        free(users);
        return;
    }
    
    if(strcmp(com, "RUL")==0 && strcmp(status, "NOK")== 0){
        printf("Non Existing Group!\n");
    }
    else if(strcmp(com, "RUL")==0 && strcmp(status, "OK")==0){
        
        if(strcmp(users, "")==0){
            printf("No users subscribed!\n");
        }
        else{
            printf("Subscribed users in %s:\n", gname);

            strcat(users, " ~");// Stop while

            while(strcmp(users, "~")!= 0){
                sscanf(users,"%d %[^\n]",&uid, users);
                printf("> %05d\n", uid);
            }
        }
    }
    else{
        printf("Unexpected error\n");
    }

        free(users);
    free(response);
}

void commandPost(char* command){
    char* response, *message, *data;
    char com[4], text[242], fileName[24];
    int tsize;
    long fsize = 0, hSize;
    int n = sscanf(command, "%s \"%[^\"]\" %s", com, text, fileName);

    strcpy(currentUID, "12345");
    currentGID = 10;

    //printf("n: %d\n", n);
    //printf("text: %s\n", text);
    // isto aceita post "hello , corrigir

    if(n < 2){
        printf("Expected post \"text\" or post \"text\" file!\n");
        return;
    }
    
    if(strcmp(fileName, "")!=0 && (data = verifyFile(fileName, &fsize)) == NULL){
        return;
    }

    tsize = strlen(text);

    if(fsize == 0){
        message = (char* )malloc(sizeof(char) * 10000);// CORRIGIR TAMANHO
        sprintf(message, "PST %s %d %d %s\n", currentUID, currentGID, tsize, text);
    } else {
        message = (char* )malloc(sizeof(char) * 100000);// CORRIGIR TAMANHO
        sprintf(message, "PST %s %d %d %s %s %ld ", currentUID, currentGID, tsize, text, fileName, fsize);
        hSize = strlen(message);
    }



    /*FILE* fp2;

    fp2 = fopen("output.jpg", "wb"); 
    fwrite(message,1,fsize + hSize,fp2);

    fclose(fp2);*/

    //printf(" msg: %s\n", message);

    response = clientSendTCP(message, fileName);

    //printf("Res: %s\n", response);
    
    free(message);
}

void commandExit(char* message){
    char* response;
    if(isLoggedIn){
        sprintf(message, "OUT %s %s\n", currentUID, currentPass);
        response = clientSendUDP(message,SIZE_STRING);
        free(response);
    }
}

void processCommands(){
    
    char* buffer=(char*) malloc(sizeof(char)*1000);
    char* com = (char*) malloc(sizeof(char)*SIZE_STRING);
    char* arg2=(char*) malloc(sizeof(char)*300);
    char* arg1=(char*) malloc(sizeof(char)*300);// HARDCODED
    int n;

    //VERIFICAR ARGUMENTOS

    while(fgets(buffer,SIZE_STRING,stdin)){
        n=sscanf(buffer,"%s %s %s",com,arg1,arg2); 
        
        if(n<1)
            continue;

        if(strcmp(com,"reg")==0){
            strcpy(buffer, "");

            if(n==3){
                sprintf(buffer, "REG %s %s\n", arg1, arg2);
                commandRegister(buffer);
            }
            else
                printf("Expected 2 arguments!\n");
        }
            
        else if(strcmp(com,"unregister")==0 || strcmp(com,"unr")==0){
            strcpy(buffer, "");

            if(n==3){
                sprintf(buffer, "UNR %s %s\n", arg1, arg2);
                commandUnregister(buffer, arg1);
            }
            else
                printf("Expected 2 arguments!\n"); 
        }
            
        else if(strcmp(com,"login")==0){
            strcpy(buffer, "");

            if(n==3){
                if(!isLoggedIn){
                    sprintf(buffer, "LOG %s %s\n", arg1, arg2);
                    commandLogin(buffer, arg1, arg2);
                }
                else
                    printf("Already logged in\n");
            }
            else
                printf("Expected 2 arguments!\n");
        }

        else if(strcmp(com,"logout")==0){
            strcpy(buffer, "");

            if(n==1){
                if(isLoggedIn){
                    sprintf(buffer, "OUT %s %s\n", currentUID, currentPass);
                    commandLogout(buffer);
                }
                else
                    printf("No login\n");
            }
            else
                printf("No arguments expected!\n");

            
        }

        else if(strcmp(com,"showuid")==0 || strcmp(com,"su")==0){
            strcpy(buffer, "");

            if(n==1){
               commandShowUID(); 
            }
            else
                printf("No arguments expected!\n");

            
        }

        else if(strcmp(com,"groups")==0 || strcmp(com,"gl")==0){
            strcpy(buffer, "");

            if(n==1){
                sprintf(buffer, "GLS\n");
                commandGroups(buffer);
            }
            else
                printf("No arguments expected!\n");

                 
        }

        else if(strcmp(com,"subscribe")==0 || strcmp(com,"s")==0){  
            strcpy(buffer, "");

            if(n==3){
                if(isLoggedIn){
                    sprintf(buffer, "GSR %s %s %s\n", currentUID, arg1, arg2);
                    commandSubscribe(buffer);
                }
                else{
                    printf("No login\n");
                }
            }
            else
                printf("Expected 2 arguments!\n");
        }    

        else if(strcmp(com,"unsubscribe")==0 || strcmp(com,"u")==0){  
            strcpy(buffer, "");

            if(n==2){
                if(isLoggedIn){
                    sprintf(buffer, "GUR %s %s\n", currentUID, arg1);
                    commandUnsubscribe(buffer);
                }
                else{
                    printf("No login\n");
                } 
            }
            else
                printf("Expected 1 arguments!\n");
        }   

        else if(strcmp(com,"select")==0 || strcmp(com,"sag")==0){  
            strcpy(buffer, "");

            if(n==2){
                commandSelect(arg1);
            }
            else
                printf("Expected 1 arguments!\n");
        }  

        else if(strcmp(com,"showgid")==0 || strcmp(com,"sg")==0){   
            strcpy(buffer, "");

            if(n==1){
                commandShowGID();
            }
            else
                printf("No arguments expected\n");
        } 

        else if(strcmp(com,"my_groups")==0 || strcmp(com,"mgl")==0){   
            strcpy(buffer, "");

            if(n==1){
                if(isLoggedIn){
                    sprintf(buffer, "GLM %s\n", currentUID);
                    commandMyGroups(buffer);
                }
                else{
                    printf("No login\n");
                } 
            }
            else
                printf("No arguments expected\n");
        }

        else if(strcmp(com,"ulist")==0 || strcmp(com,"ul")==0){   
            strcpy(buffer, "");

            if(n==1){
                if( currentGID != 0){
                    sprintf(buffer, "ULS %02d\n", currentGID);
                    commandUList(buffer);
                } 
                else{
                    printf("No group selected\n");
                }
            }
            else
                printf("No arguments expected\n");
        }

        else if(strcmp(com,"post") == 0){  

            commandPost(buffer);

            if(!isLoggedIn){
                printf("Not logged in\n");
            }
            else if(currentGID == 0){
                printf("No group selected\n");
            }
            else{
                commandPost(buffer);
            }

            strcpy(buffer, "");
        }

        else if(strcmp(com,"exit")==0){

            if(n==1){
                commandExit(buffer);
                free(buffer);
                free(com);
                free(arg1);
                free(arg2);
                return;
            }
            else
                printf("No arguments expected\n");
            
        }

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
    
    free(currentPass);
    free(currentUID);
    free(DSport);
    free(DSIP);

    return 0;
}