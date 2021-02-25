
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "server.h"
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
// ./server -r ROOT     -p  PORT  -n THREAD_NUMBER
// ./server -r testdir  -p  12345 -n 4
//run -r testdir -p 10000 -n 4
typedef struct//argument to child
{
    char *root;
    char *str;
    char *higher;
} arg;
typedef struct request
{
    char buf[256];
    int socket;
    struct request *next;
};
typedef struct job//jobqueue
{
    int count;
    struct request *head;
    struct request *tail;
};

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
int forClientSockfd = 0, isdir = 0 ;
char msg[256] = {""};
char root[256] = {"./"};
//socket的建立
int sockfd = 0;
struct job j;

void substr(char *dest, const char* src, unsigned int start, unsigned int cnt);
void int2str(int i, char *s);
void printdir(char *dir, char *str, char *higher);
int look(char *str,char *index);
void fail();

void* child()
{
    while(1)
    {
        //printf("L\n");
        struct request *r = NULL;
        pthread_mutex_lock( &mutex1 );
        //printf("P\n");
        //while(j.count==0);
        if(j.count != 0)
        {
            if(j.head->next == NULL) j.tail = NULL;
            r = j.head;
            j.head = j.head->next;
            j.count--;
        }
        pthread_mutex_unlock( &mutex1 );
        if(r!=NULL)
        {
            char buf[256];
            strcpy(buf,r->buf);
            free(r);

            char *token = "",*buff = "",bufprint[256] = {};
            char delim[] = " ";
            char str[256]="";
            int ndq = 0;


            buff=buf;
            strcpy(bufprint,buf);
            token=strtok(&buff,delim);
            char isquery[8] = "Query ";
            int notquery = 0;
            for(int i=0; i<6; i++)
            {
                if(isquery[i]!=buf[i])
                {
                    fail();
                    notquery =1 ;
                    break;
                }
            }
            if(notquery==1)
                continue;

            strcpy(buf,buff);//get string excpet Query

            ndq = 0;//number of double quote
            int j=0,len=0,q=0;
            for(int i = 0; i < strlen(buf); i++)
            {

                if(buf[i]=='\"')
                    ndq++;
                else if(ndq % 2 == 1)
                    str[j++] = buf[i];

                if(ndq % 2 == 0 && ndq > 0)
                {
                    str[j]='\0';
                    strcpy(msg,"String: \"");
                    strcat(msg,str);
                    strcat(msg,"\"\n");
                    send(forClientSockfd,msg,sizeof(msg),0);
                    printdir(root,str,"");
                    if(isdir==0)
                    {
                        strcpy(msg,"Not found\n");
                        send(forClientSockfd,msg,sizeof(msg),0);
                    }

                    isdir = 0;
                    ndq = 0;
                    j = 0;
                    q = 1; //query 數量
                }
            }
            if(q!=1)
            {
                fail();
            }
            else
                printf("%s\n",bufprint);


            strcpy(msg,"vv");//所有request 結束
            strcpy(buff,"");
            send(forClientSockfd,msg,sizeof(msg),0);
            strcpy(msg,"");
            isdir = 0;
        }
    }
}

int main(int argc, char *argv[])
{
    int nth=0;//當前用到第幾個thread
    int port = 0;
    int nthread = 0;
    strcat(root,argv[2]);
    port = atoi(argv[4]);
    nthread = atoi(argv[6]);
    pthread_t pth[nthread];
    pthread_t p;

    char str[256];

    for(int i=0; i<nthread; i++)
    {
        pthread_create(&p, NULL, child, NULL);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Fail to create a socket.");
    }
    //socket的連線
    struct sockaddr_in serverInfo,clientInfo;
    unsigned int addrlen = sizeof(clientInfo);
    memset(&serverInfo,0,sizeof(serverInfo));

    serverInfo.sin_family = PF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(port);
    bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
    listen(sockfd,5);
    forClientSockfd = accept(sockfd,(struct sockaddr*) &clientInfo, &addrlen);


    j.count=0;
    j.head=NULL;
    j.tail=NULL;

    while(1)
    {
        int back=0;
        back=recv(forClientSockfd,str,sizeof(str),0);
        if(back<=0)
        {
            forClientSockfd = accept(sockfd,(struct sockaddr*) &clientInfo, &addrlen);
            continue;
        }
        else
        {
            if(str!=NULL)
            {
                struct request* temp;
                temp=malloc(sizeof(struct request));
                strcpy(temp->buf,str);
                temp->socket=forClientSockfd;
                if(j.count==0)
                {
                    j.head=temp;
                    j.tail=temp;
                    j.head->next=NULL;
                    j.tail->next=NULL;
                    j.count++;
                }
                else
                {
                    j.tail->next=temp;
                    j.tail=j.tail->next;
                    j.count++;
                }
            }
        }
    }


    close(sockfd);
    return 0;
}

void int2str(int i, char *s)
{
    sprintf(s,"%d",i);
}
int look(char *str,char *index)
{
    int count=0;
    if(*index=='\0') return 0;
    char *buf = strstr(str, index);
    while(buf != NULL)
    {
        str = buf + 1;
        buf = strstr(str, index);
        count++;
    }
    return count;
}
void printdir(char *dir, char *str, char *higher)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    char msg[256];
    char high[256];

    if((dp = opendir(dir)) == NULL)
    {
        fprintf(stderr,"cannot open directory: %s\n", dir);
        return;
    }
    chdir(dir);
    while((entry = readdir(dp)) != NULL)
    {
        lstat(entry->d_name,&statbuf);

        //strcpy(msg,"File: ./");
        //send(forClientSockfd,msg,sizeof(msg),0);

        if(S_ISDIR(statbuf.st_mode))
        {
            /* Found a directory, but ignore . and .. */
            if(strcmp(".",entry->d_name) == 0 ||
                    strcmp("..",entry->d_name) == 0)
                continue;
            //printf("%s/\n",entry->d_name);//印資料夾名稱

            /* Recurse at a new indent level */
            //strcpy(msg,entry->d_name);
            //send(forClientSockfd,msg,sizeof(msg),0);
            strcpy(high,higher);
            strcat(high,entry->d_name);
            strcat(high,"/");
            printdir(entry->d_name,str,high);
        }
        else
        {
            FILE *fptr;
            char f[1024]= {};
            char n[11]= {};
            int num=0;

            strcpy(msg,"File: ./");
            strcat(msg,higher);

            strcat(msg,entry->d_name);

            if ((fptr = fopen(entry->d_name, "r")) == NULL)
            {
                printf("open_file_error");
                exit(1);
            }
            fgets(f, 1024, fptr);
            num = look(f, str);
            if(num==0)
            {
                fclose(fptr);///若無相等則不須回傳
                continue;
            }
            else
            {
                strcat(msg,", Count: ");
                int2str(num,n);
                strcat(msg,n);
                strcat(msg,"\n");
                send(forClientSockfd,msg,sizeof(msg),0);
                isdir++;
                fclose(fptr);
            }

        }

    }
    chdir("..");
    closedir(dp);
}
void substr(char *dest, const char* src, unsigned int start, unsigned int cnt)
{
    strncpy(dest, src + start, cnt);
    dest[cnt] = 0;
}
void fail()
{
    strcpy(msg,"The string format is not correct\n");//無任何雙引號 格式錯誤
    send(forClientSockfd,msg,sizeof(msg),0);
}