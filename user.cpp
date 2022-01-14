#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/stat.h>
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
    tmout.tv_sec=15; /* Wait for 15 sec for a reply from the server. */
    return(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tmout,sizeof(struct timeval)));
}

int TimerOFF(int sd){
    struct timeval tmout;
    memset((char *)&tmout,0,sizeof(tmout)); /* clear time structure */
    return(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tmout,sizeof(struct timeval)));
}



void verifyArguments(int numArgs, char *args[]){ //verifies the validity of the flags given upon running the program 
    if (numArgs != 1 && numArgs != 3 && numArgs != 5){
        fprintf(stderr,"Invalid number of Arguments!");
        exit(EXIT_FAILURE);
    }
    return; 
}

void getDefaultIP(){ //gets the default hostname, if none is specified upon running the program

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
            strcpy(DSIP, inet_ntop(p->ai_family,addr,buffer,sizeof buffer)); //places hostname in global variable DSIP
        }
        freeaddrinfo(res);
    }
}

void getDefaultPort(){ //gets the default hostname, if none is specified upon running the program
    int port = DEFAULT_PORT + GROUP_NUMBER; //58000 + Group Number
    sprintf(DSport, "%d", port);   
}


void parseArgs(int numArgs,char *args[]){ //parses the arguments given upon running the program
    if (numArgs == 1){ //no flags given
        getDefaultIP();
        getDefaultPort();
    }
    for(int i=1;i<numArgs;i+=2){
        if(strcmp(args[i],"-n") == 0){ // if -n was given
            strcpy(DSIP, args[i+1]);
            if(numArgs < 5)
                getDefaultPort();
        }
        else if(strcmp(args[i],"-p") == 0){ //if -p was given
            strcpy(DSport ,args[i+1]);
            if(numArgs < 5)
                getDefaultIP();
        }
    }
}
char* clientSendUDP(char* message, int sizeString){ //sends & receives a message with given sizeString through a UDP socket
    
    int fd,errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;
    char* response;
    
    
    if((fd=socket(AF_INET,SOCK_DGRAM,0)) == -1) 
        exit(EXIT_FAILURE); //socket creation error


    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;

    if((errcode=getaddrinfo(DSIP,DSport,&hints,&res)) != 0)
        exit(EXIT_FAILURE); //address info error
    
    if((n=sendto(fd,message,strlen(message),0,res->ai_addr,res->ai_addrlen))==-1) //sends messages to server
        exit(EXIT_FAILURE); //error sending messages

    addrlen=sizeof(addr);

    response = (char*) malloc(sizeof(char)* sizeString);
    memset(response, 0, sizeof(response));

    TimerON(fd); //starts a 15 sec timer
    n=recvfrom(fd,response,sizeString,0,(struct sockaddr*)&addr,&addrlen); //receives messages from server

    sscanf(response, "%[^\n]", response); //assures that the response buffer is properly formatted
    strcat(response, "\n");

    if(errno == EAGAIN){  
        printf("Timeout occured!\n");
    }
    else if(n == -1)
        exit(EXIT_FAILURE);
    TimerOFF(fd);

    freeaddrinfo(res);
    close(fd);
    return response;
}

void fileSendTCP(char* filename, long fsize, int fd){ //sends a file with given fsize through a TCP socket

    

    FILE* fp = fopen(filename, "rb");
    char* buffer = (char*) malloc(sizeof(char)*1024);
    long toSend, n;

    bzero(buffer, 1024); 

    toSend = fsize;
    while(toSend > 0) {
        n = fread(buffer, 1, 1024, fp);
        
        while((n = write(fd, buffer, n)) == -1){}
        toSend-= n;
        bzero(buffer, 1024);
    }
    free(buffer);
    return;
}

char* clientSendTCP(char* message, char* fileName, long fsize){

    

    int fd, errcode;
    ssize_t n = 1;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char *response, *ptr;
    int i = 1, nread = 0;
    int messageSize;

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
    messageSize= strlen(message);

    while(messageSize > 0){
        n=write(fd, ptr, messageSize);

        if(n<=0)
            exit(1);

        messageSize-= n;
        ptr+= n;
    }
    
    if(fileName != NULL)
        fileSendTCP(fileName, fsize, fd);
    
    response = NULL;

    while(n > 0){ //sends a file if asked
        response =(char*) realloc(response, sizeof(char) * ((i * 256) + 1));

        if(i == 1){
            memset(response, 0, sizeof(response));
        }

        n=read(fd, response + nread, 256);
        
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

char* verifyFile(char fileName[], long* fsize){ //verifies a file with given fileName & size fsize, returning the file's contents upon success
    
    FILE *fp;
    int nameSize = strlen(fileName);
    char* data;

    for(int i = 0; i < nameSize; i++){ //Verify every char of fileName
        if(!isalpha(fileName[i]) && !isdigit(fileName[i]) && fileName[i] != '_' && fileName[i] != '-' && fileName[i] != '.'){
            printf("Invalid file name\n");
            return NULL;
        }
    }

    //read file
    if((fp = fopen(fileName, "r")) != NULL){
        
        fseek(fp, 0, SEEK_END); //getting file size 
        *fsize = ftell(fp); 
        fseek(fp, 0, SEEK_SET); 

        data = (char*) malloc(*fsize);
        
        if(fread(data,1,*fsize,fp) != *fsize){ //doesn't distinguish between EOF and read errors
            printf("Reading error\n");
            return NULL;
        }
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

void commandRegister(char* message){ //executes the register command
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

void commandUnregister(char* message, char* UID){ //executes the unregister command
    char* response = clientSendUDP(message,SIZE_STRING);

    if(strcmp("ERR\n",response)==0)
        fprintf(stderr,"Un-Registration error\n");

    else if(strcmp("RUN OK\n",response)==0) { 
        printf("Accepted Un-Registration!\n");
        if(isLoggedIn && strcmp(currentUID,UID)==0){ //locally logs out the user
            isLoggedIn=false;
        }
    }
    else if(strcmp("RUN NOK\n",response)==0)
        printf("Not Accepted Un-Registration!\n"); 
    else
        printf("Unexpected error\n");
    
    free(response);
}

void commandLogin(char* message, char* UID, char* pass){ //executes the login command
    char* response = clientSendUDP(message,SIZE_STRING);

    if(strcmp("ERR\n",response)==0){
        fprintf(stderr,"Login error\n");
    }
    else if(strcmp("RLO OK\n",response)==0){  //locally updates the current UID & password
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

void commandLogout(char* message){ //executes the logout command
    char* response = clientSendUDP(message,SIZE_STRING);

    if(strcmp("ERR\n",response)==0){
        fprintf(stderr,"Logout error\n");
    }
    else if(strcmp("ROU OK\n",response)==0){  //locally clears the current UID & password
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

void commandShowUID(){ //executes the showuid command
    if(isLoggedIn) //locally checks if the user is logged in 
        printf("User ID: %s \n", currentUID);
    else 
        printf("Not logged in!\n");   
}

void commandGroups(char* message){ //executes the groups command
    char* response = clientSendUDP(message,SIZE_GROUPS);  
    char com[4], gname[24];
    int numGroups, n, gid, mid;

    if(strcmp("ERR\n",response)==0){
        fprintf(stderr,"Group error\n");
        free(response);
        return;
    }
    
    n=sscanf(response,"%s %d %[^\n]",com, &numGroups, response); 

    if(n < 3){ //if the number of arguments read in incorrect
        printf("Unexpected error\n");
        free(response);
        return;
    }
    if(strcmp(com, "RGL")==0 && numGroups == 0){ //no groups to be shown
        printf("No Groups Available!\n");
    }
    else if(strcmp(com, "RGL")==0 && numGroups > 0 && numGroups <= 99){ //there are groups to be shown
        printf("Available Groups:\n");
        while(numGroups != 0){
            sscanf(response,"%d %s %d %[^\n]",&gid, gname, &mid,response);
            printf("> %02d: %s [msg: %04d]\n", gid, gname, mid);
            numGroups--;
        }
    }
    else{
        printf("Unexpected error\n");
    }
    free(response);
}

void commandSubscribe(char* message){ //executes the subscribe command
    char  GID[3]; 
    
    char* response = clientSendUDP(message,SIZE_STRING);
    
    if(strcmp("ERR\n",response)==0){
        fprintf(stderr,"Subscribe error\n");
    }
    else if(strcmp("RGS OK\n", response) == 0){
        printf("Accepted Subscription!\n");
    }
    else if(sscanf(response, "RGS NEW %s", GID) == 1){ //if a new group was created, displays it to the user
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

void commandUnsubscribe(char* message){ //executes the unsubscribe command
    
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

void commandSelect(char* GID){ //executes the select command
    if (atoi(GID) > 0 && atoi(GID) < 100){ //locally updates the GID if it's valid
        currentGID = atoi(GID); //only updates the global variable if everything is correct
        printf("Selected Group ID: %d \n", currentGID);
    }
    else 
        printf("Invalid Group ID!\n");
}

void commandShowGID(){ //executes the showgid command
    if (currentGID != 0){ //if the GID is valid, display it
        printf("Current Group ID: %d \n", currentGID);
    }
    else 
        printf("No group selected!\n");
}

void commandMyGroups(char* message){ //executes the mgl command
    char* response;
    char com[COM_SIZE], groupName[SIZE_GROUP_NAMES];
    int numGroups, n, gid, mid;

    response = clientSendUDP(message,SIZE_GROUPS); 
        
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
    
    n=sscanf(response,"%s %d %[^\n]",com, &numGroups, response); 
    if(n < 2){
        printf("Unexpected error\n");
        free(response);
        return;
    }

    if(strcmp(com, "RGM")==0 && numGroups == 0){
        printf("No Groups Subscribed To!\n");
    }
    else if(strcmp(com, "RGM")==0 && numGroups > 0 && numGroups <= MAX_GROUPS){ //if everything is valid, displays the groups which the user is subscribed to 
        printf("My Groups:\n");
        
        while(numGroups != 0){
            sscanf(response,"%d %s %d %[^\n]",&gid, groupName, &mid,response);
            printf("> %02d: %s [msg: %04d]\n", gid, groupName, mid);
            numGroups--;
        }
    }
    else{
        printf("Unexpected error\n");
    }

    free(response);
}

void commandUList(char* message){ //executes the ul command
    char* response = clientSendTCP(message, NULL, 0);
    char* users = (char *) malloc(sizeof(char) * strlen(response));
    char com[COM_SIZE], groupName[SIZE_GROUPS], status[COM_SIZE];
    int n, uid;


    memset(users, 0, sizeof(users));
    if(strcmp("ERR\n",response)==0){
        fprintf(stderr,"UList error\n");
        free(response);
        return;
    }

    n=sscanf(response,"%s %s %s %[^\n]",com, status, groupName, users);

    if(n < 2){
        printf("Unexpected error\n");
        free(response);
        free(users);
        return;
    } 
    if(strcmp(com, "RUL") == 0 && strcmp(status, "NOK") == 0){
        printf("Non Existing Group!\n");
    }
    else if(strcmp(com, "RUL") == 0 && strcmp(status, "OK") == 0){
        
        if(strcmp(users, "") == 0){
            printf("No users subscribed!\n");
        }
        else{
            printf("Subscribed users in %s:\n", groupName);
            strcat(users, " ~"); // the tilde is used as a signal to stop the following while
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

void commandPost(char* command){ //executes the post command
    char* response, *message, *data;
    char com[COM_SIZE], text[MAX_TEXT_SIZE], fileName[SIZE_GROUP_NAMES]; 
    int tsize;
    int msg = -1;
    long fsize = 0;

    int n = sscanf(command, "%s \"%[^\"]\" %s", com, text, fileName);

    if(n < 2){
        printf("Expected post \"text\" or post \"text\" file!\n");
        return;
    }
    if(strcmp(fileName, "")!=0 && (data = verifyFile(fileName, &fsize)) == NULL){ //if a fileName was given but its contents are unreadable
        return;
    }

    tsize = strlen(text); //getting the text size

    if(fsize == 0){ //if no file was sent
        message = (char*) malloc(sizeof(char) * (19+tsize));// command size + text size
        memset(message, 0, sizeof(message));
        sprintf(message, "PST %s %02d %d %s\n", currentUID, currentGID, tsize, text);
        response = clientSendTCP(message, NULL, 0);
    } else { //file was sent
        message = (char*) malloc(sizeof(char) * (18+tsize+strlen(fileName)+13));// command size + text size + fileName size + length of fSize
        memset(message, 0, sizeof(message));
        sprintf(message, "PST %s %02d %d %s %s %ld ", currentUID, currentGID, tsize, text, fileName, fsize);
        response = clientSendTCP(message, fileName, fsize);
    }
    
    sscanf(response,"%s %d\n",com,&msg);

    if(strcmp(response,"RPT NOK")==0)
        printf("Unsuccessful Post\n");
    else if(strcmp(com,"RPT") == 0 && msg != -1)
        printf("Messages successfully posted! (MID: %d)\n", msg); //display message ID upon success
    else if(strcmp(response,"ERR")==0)
        printf("POST error\n");
    else
        printf("Unexpected error\n");

    free(message);
}

int getDigits(int m){ //auxiliary function that gets the length in digits of a certain integer m
    int d = 0;

    while(m > 0){
        m = m / 10;
        d++;
    }

    return d;
}

void commandRetrieve(char* command){ //executes the retrieve command
    char* response = clientSendTCP(command, NULL, 0);
    char* responseAux, *fileDir;
    char com[COM_SIZE], status[COM_SIZE], mid[5], uid[6], fileName[25];
    int numMSG, n, tsize;
    long fsize;
    FILE* fp;

    responseAux = response; //responseAux will be the pointer that we will use in order to read the message sent by the server

    memset(status, 0, sizeof(status));
    memset(com, 0, sizeof(com));

    printf("%s\n", response);

    if(strcmp("ERR\n",response)==0){
        fprintf(stderr,"Retrieve error\n");
        free(response);
        return;
    }

    n = sscanf(response, "%s %s %d", com, status, &numMSG);

    if(n < 2){ //incorrect number of arguments
        printf("Unexpected error\n");
        free(response);
        return;
    }

    if(strcmp(com, "RRT") == 0){
        if(strcmp(status, "OK") == 0){

            responseAux += (8 + getDigits(numMSG));

            while(numMSG > 0){
                memset(mid, 0, sizeof(mid));
                memset(uid, 0, sizeof(uid));
                
                n = sscanf(responseAux, "%s %s %d", mid, uid, &tsize);

                if(n < 3){
                    printf("Unexpected error\n");
                    free(response);
                    return;
                }

                responseAux += (12 + getDigits(tsize));
                printf("%s(User: %s) > %.*s\n", mid, uid, tsize, responseAux);

                responseAux += tsize;

                if(responseAux[0] == '\n'){
                    break;
                }

                responseAux += 1;

                if(responseAux[0] == '/'){
                    fileDir = (char*) malloc(sizeof(char) * 50);
                    memset(fileName, 0, sizeof(fileName));
                    memset(fileDir, 0, sizeof(fileDir));

                    responseAux += 2;

                    n = sscanf(responseAux, "%s %ld", fileName, &fsize);

                    if(n < 2){
                        printf("Unexpected error\n");
                        free(response);
                        free(fileDir);
                        return;
                    }
                    
                    responseAux += (strlen(fileName) + getDigits(fsize) + 2);

                    mkdir("Received_Message_Files", 0700);
                    sprintf(fileDir, "Received_Message_Files/%s", mid);
                    mkdir(fileDir, 0700);
                    sprintf(fileDir, "Received_Message_Files/%s/%s", mid, fileName);

                    fp = fopen(fileDir, "w+");
                    fwrite(responseAux, 1, fsize, fp);
                    fclose(fp);

                    printf("    Attached File in : %s (%ldB)\n", fileDir, fsize);

                    responseAux += fsize;

                    if(responseAux[0] == '\n'){
                        free(fileDir);
                        break;
                    }

                    responseAux += 1;
                    free(fileDir);
                }
                numMSG--;
            }
        }
        else if(strcmp(status, "NOK") == 0)
            printf("Retrieve request error\n");    
        else if(strcmp(status, "EOF") == 0)
            printf("No messages to be printed!\n");
    } 
    else
        printf("Unexpected Error\n");
    
    free(response);
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
    char* arg1=(char*) malloc(sizeof(char)*300);
    int n;

    //VERIFICAR ARGUMENTOS

    while(fgets(buffer,1000,stdin)){
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

        else if(strcmp(com, "retrieve") == 0 || strcmp(com, "r") == 0){
            strcpy(buffer, "");

            if(n==2){
                if(!isLoggedIn){
                    printf("Not logged in\n");
                }
                else if(currentGID == 0){
                    printf("No group selected\n");
                }
                else{
                    sprintf(buffer, "RTV %s %02d %s\n", currentUID, currentGID, arg1);
                    commandRetrieve(buffer);
                }
            }
            else
                printf("Expected 1 arguments!\n");
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