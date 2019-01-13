#include "task.h"

int clienttask::MAXDATASIZE = 50;
int clienttask::PORT = 2222; 
bool clienttask::Run(){
    
    int sockfd, num;
    char buf[MAXDATASIZE];
    struct hostent *he;
    struct sockaddr_in server;
    char *server_ip = "127.0.0.1";
    if((he=gethostbyname(server_ip))==NULL){
        printf("gethostbyname() error\n");
        return false;
    }
    if((sockfd=socket(AF_INET,SOCK_STREAM, 0))==-1){
        printf("socket() error\n");
        return false;
    }

    bzero(&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr = *((struct in_addr *)he->h_addr);
    if(connect(sockfd, (struct sockaddr *)&server, sizeof(server))==-1)
    {
        printf("connect() error\n");
        return false;
    }
    //printf("connect success\n"); 
    char str[] = "hello my babe!";

    if((num=send(sockfd,str,sizeof(str),0))==-1){
        printf("send() error\n");
        return false;
    }
    //printf("client->server mesg is: %s\n", str);
    num=recv(sockfd,buf,MAXDATASIZE,0);
    if(num == -1)
    {
        printf("recv() error\n");
        if( errno == ECONNRESET )
        {
            printf("ECONNRESET\n");
        }
        else if( errno == EAGAIN ) 
        {
            printf("EAGAIN\n");
        }
        else if( errno == EWOULDBLOCK )
        {
            perror("recv not over...\n");
        }
        else
        {
            printf("else\n");
        }
        return false;
    }     

    buf[num-1]='\0';
    close(sockfd);
    usleep(20*1000);
    return true;
}
