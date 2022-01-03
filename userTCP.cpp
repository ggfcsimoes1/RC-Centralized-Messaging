#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "userTCP.hpp"

void clientSendTCP(){
    int fd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char buffer[128];
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd==-1) exit(1);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    errcode=getaddrinfo("tejo.tecnico.ulisboa.pt", PORT, &hints, &res);
    if(errcode!=0) exit(1);

    n = connect(fd, res-> ai_addr, res-> addrlen);
    if(n==-1) exit(1);

    n=write(fd, "Hello!\n", 7);
    if(n==-1) exit(1);

    n=read(fd,buffer, 128);
    if(n==-1) exit(1);

    write(1, "echo: ", 6);
    write(1, buffer, n);

    freeaddrinfo(res);
    close(fd);
}