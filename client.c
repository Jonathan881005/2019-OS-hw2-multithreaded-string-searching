#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
// ./client -h LOCALHOST -p PORT
// ./client -h 127.0.0.1 -p 12345
//run -h 127.0.0.1 -p 10000
int sockfd = 0;
int port;
char localhost[50]="";
char str[256] = {};
char buf[256] = {};


void query(void *arg)
{
    //socket的建立
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)//socket的連線
    {
        printf("Fail to create a socket.");
    }

    struct sockaddr_in info;
    memset(&info,0,sizeof(info));
    info.sin_family = PF_INET;
    //localhost test
    info.sin_addr.s_addr = inet_addr(localhost);
    info.sin_port = htons(port);

    int err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
    if(err==-1)
    {
        printf("Connection error\n");
    }

    //Send a message to server
    //printf("%s",buf);


    str[strlen(str)-1]='\0';

    send(sockfd,str,sizeof(str),0);
    strcpy(str, "");
    while(recv(sockfd,buf,sizeof(buf),0))
    {
        if(strcmp(buf,"vv")==0)
            break;
        else
            printf("%s",buf);
        strcpy(str, "");
    }


    close(sockfd);
}
int main(int argc, char *argv[])
{
    strcat(localhost,argv[2]);
    port = atoi(argv[4]);
    while(1)
    {
        fgets(str,256,stdin);
        pthread_t pth;
        pthread_create(&pth,NULL,query,(void*)str);
    }


    return 0;
}
