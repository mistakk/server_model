#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>

#include <iostream>
#define PORT 2222
#define CLIENTNUM 4000
#define BACKLOG 10
#define MAXRECVLEN 50
using namespace std;
void printids(const char *s);
static void *handle_request(void *argv);
static void *handle_connect(void *argv);
int connectfd_list[CLIENTNUM];
int port_list[CLIENTNUM];
bool shutflag = false;
struct timeval tv, tv_first, tv_new;
void handler(int a);

char* transtime(struct timeval tv){
    char time_string[40];
    long milliseconds;
    struct tm* ptm = localtime(&tv.tv_sec);
    strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", ptm);
    milliseconds = tv.tv_sec / 1000;
    //printf("%s.%03ld\n", time_string, milliseconds);
    return time_string;
}
int main(int argc, char *argv[])
{
    signal(SIGUSR1, handler);
    int listenfd;   /* socket descriptors */
    struct sockaddr_in server; /* server's address information */
    socklen_t addrlen;
    /* Create TCP socket */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        /* handle exception */
        perror("socket() error. Failed to initiate a socket");
        exit(1);
    }
 
    /* set socket option */
    int opt = SO_REUSEADDR;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listenfd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        /* handle exception */
        perror("Bind() error.");
        exit(1);
    }
    
    if(listen(listenfd, BACKLOG) == -1)
    {
        perror("listen() error. \n");
        exit(1);
    }

    pthread_t thread_do[2];
    pthread_create(&thread_do[0], NULL, handle_connect, (void*)&listenfd);
    pthread_create(&thread_do[1], NULL, handle_request, NULL);

    int i;
    for(i = 0; i<2; i++){
        pthread_join(thread_do[i], NULL);    
    }

    close(listenfd);
    return 0;
}

static void *handle_connect(void* argv){
    printids("[thread_connect] success start!");
    int listenfd = *((int*)argv);
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    int server_times = 0;
    int i, connectfd;
    for(i = 0;i<CLIENTNUM;i++)
        connectfd_list[i] = -1;
    while(1){
        gettimeofday(&tv, NULL);
        if(shutflag){
            printf("[%s] [thread_connect] connection thread got signal to exit;\n", transtime(tv));
            break;
        }  
        if((connectfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1){
            perror("accpet error.\n");
            exit(1);
        }
        server_times ++;
        //printf("[%s] [thread_connect] request, client ip %s, port %d, req_cnt %d\n", transtime(tv), inet_ntoa(client.sin_addr),htons(client.sin_port), server_times);
        bool success_connection = false;
        for(i = 0;i<CLIENTNUM;i++){
            if(connectfd_list[i] == -1){
                success_connection = true;
                connectfd_list[i] = connectfd;
                port_list[i] = htons(client.sin_port);
                break;
            }    
        }
        if(success_connection == false)
            printf("[%s] [thread_connect] failed, for have no free seat\n" , transtime(tv));
    }
}

static void *handle_request(void *argv){
    int speaktimes = 0;
    gettimeofday(&tv, NULL);
    gettimeofday(&tv_first, NULL);
    printids("[thread_request] success start!");
    char buf[MAXRECVLEN];
    
    struct timeval timeout;
    fd_set readfds;
    int connectfd;
    int maxfd;
    int retval;
    int i;
    bool waitting = false;
    bool flag_stop = false;


    while(1){
        if(shutflag){
            printf("[%s] [thread_request] request thread got signal to exit;\n", transtime(tv));
            break;
        }
        FD_ZERO(&readfds);
        maxfd = 0;
        for(i = 0;i < CLIENTNUM; i++){
            if(connectfd_list[i]!=-1){
                FD_SET(connectfd_list[i], &readfds);
                maxfd = connectfd_list[i];
            }
        }
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        retval = select(maxfd+1, &readfds, NULL, NULL, &timeout);
        gettimeofday(&tv, NULL);
        if(retval < 0)
            perror("error\n");
        else if (retval ==0){
            int timegap_to_last_do = tv.tv_sec - tv_first.tv_sec;
            if(!waitting && timegap_to_last_do > 3){ 
                int milliseconds = (tv.tv_sec - tv_first.tv_sec);
                printf("[%s] [thread_request] [alarm] Waitting, before process %d times, cost %d s;\n", transtime(tv), speaktimes, milliseconds);
                waitting = true;
                speaktimes = 0;
            }
            continue;
        }
        if(waitting == true){
            waitting = false;
            gettimeofday(&tv_first, NULL);
        }
        for(i = 0;i < CLIENTNUM; i++){
            gettimeofday(&tv, NULL);
            connectfd = connectfd_list[i];
            if(connectfd!=-1 && FD_ISSET(connectfd, &readfds)){
                int iret = -1;
                while(1){
                    memset(buf, 0, MAXRECVLEN);
                    iret = recv(connectfd, buf, MAXRECVLEN, 0);
                    if(iret > 0){
                        ;//printf("[%s] [thread_request] [message] client[%d]->server len is: %d, mesg is: %s\n", transtime(tv), port_list[i], iret, buf);
                    }
                    else{
                        close(connectfd);
                        connectfd_list[i] = -1;
                        break;
                    }
                    speaktimes += 1;
                    sprintf(buf, "Hi~I got your message, it's my %d times send msg;", speaktimes);
                    send(connectfd, buf, strlen(buf), 0);
                    if(speaktimes %10000 ==0)
                        printf("[%s] [thread_request] [message] server->client[%d] len is %d, mesg is: %s\n", transtime(tv), port_list[i], strlen(buf), buf);
                }
            }
        }
    }
    printf("[%s] [thread_request] [alarm] server send message %d times;\n", transtime(tv), speaktimes);
    return NULL;
}
void handler(int sig){
    if(sig == SIGUSR1)
        shutflag = true;
}
void printids(const char *s){
    pid_t pid;
    pthread_t tid;
    
    pid = getpid();
    tid = pthread_self();
    printf("%s pid %lu tid %lu (0x%lx)\n", s, (unsigned long)pid, (unsigned long)tid, (unsigned long)tid);    

}
