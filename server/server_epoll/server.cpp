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
#include <sys/epoll.h> 
#include <fcntl.h>

#include <iostream>
#define PORT 2222
#define BACKLOG 10
#define MAXRECVLEN 50
using namespace std;
void printids(const char *s);
static void *handle_request(void *argv);
static void *handle_connect(void *argv);
struct epoll_event *revs;//epoll size maxnum
struct epoll_event _ev;
bool shutflag = false;
int epfd, maxnum;
struct timeval tv, tv_first, tv_new;
void handler(int a);

//设置socket连接为非阻塞模式  
void setnonblocking(int sockfd) {  
    int opts;  
  
    opts = fcntl(sockfd, F_GETFL);  
    if(opts < 0) {  
        perror("fcntl(F_GETFL)\n");  
        exit(1);  
    }  
    opts = (opts | O_NONBLOCK);  
    if(fcntl(sockfd, F_SETFL, opts) < 0) {  
        perror("fcntl(F_SETFL)\n");  
        exit(1);  
    }  
}  
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
    maxnum = 256;//default epoll get
    if(argc >=2){
        maxnum = atoi(argv[1]);
    }
    printf("server maxnum is %d;\n", maxnum);
    epfd = epoll_create(maxnum);//default is 256


    revs = (struct epoll_event*)malloc(sizeof(struct epoll_event)*maxnum);
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
    pthread_create(&thread_do[1], NULL, handle_request, (void*)&maxnum);
    for(int i = 0; i<2; i++){
        pthread_join(thread_do[i], NULL);    
    }

    if(revs !=NULL){
        free(revs); 
        revs = NULL;
    }
    close(listenfd);
    close(epfd);
    return 0;
}

static void *handle_connect(void* argv){
    printids("[thread_connect] success start!");
    int listenfd = *((int*)argv);
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    int i, connectfd, retselect, connecttimes = 0;
    fd_set listenfds;
    FD_ZERO(&listenfds);
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
        connecttimes +=1;
        //printf("[%s] [thread_connect] request, client ip %s, port %d, req_cnt %d\n", transtime(tv), inet_ntoa(client.sin_addr),htons(client.sin_port), server_times);
        
        setnonblocking(connectfd);
        _ev.events = EPOLLIN | EPOLLET;
        _ev.data.fd = connectfd;
        if(epoll_ctl(epfd, EPOLL_CTL_ADD, connectfd, &_ev) == -1)
            printf("[%s] [thread_connect] failed, for have no free seat\n" , transtime(tv));
    }
    printf("[%s] [thread_connect] [alarm] server send message %d times;\n", transtime(tv), connecttimes);
    return NULL;
}

static void *handle_request(void *argv){
    int speaktimes = 0;
    gettimeofday(&tv, NULL);
    gettimeofday(&tv_first, NULL);
    printids("[thread_request] success start!");
    char buf[MAXRECVLEN];
    
    int timeout = 1;
    fd_set readfds;
    int connectfd;
    int retval, iret;
    bool waitting = false;
    bool flag_stop = false;

    maxnum = *((int*)argv);
    if(epfd <0){
        perror("epoll_create;\n");
        exit(0);
    }


    while(1){
        if(shutflag){
            printf("[%s] [thread_request] request thread got signal to exit;\n", transtime(tv));
            break;
        }
        
        retval = epoll_wait(epfd, revs, maxnum, timeout);
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
        
        for(int i = 0; i< retval; ++i){
            int rsock = revs[i].data.fd;
            if(revs[i].events & EPOLLIN){//can be read
                iret = read(rsock, buf, MAXRECVLEN-1);
                if(iret <0)             
                    perror("read data from client error;\n");
                else if(iret ==0){
                    printf("client:%d closed;\n", rsock);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, rsock, NULL);
                    close(rsock);
                }
                else{
                    //printf("haha. i got you mesg;\n");
                    _ev.data.fd = rsock;
                    _ev.events = EPOLLOUT | revs[i].events;//add to readylist again
                    epoll_ctl(epfd, EPOLL_CTL_MOD, rsock, &_ev);
                    //_ev.events = EPOLLOUT | EPOLLET ;
                    //epoll_ctl(epfd, EPOLL_CTL_DEL, rsock, &_ev);//del
                }
            }
            else if(revs[i].events & EPOLLOUT){//in ready list, lets response something
                speaktimes += 1;
                sprintf(buf, "Hi~I got your message, it's my %d times send msg;", speaktimes);
                    //printf("haha. i send you mesg;\n");
                send(rsock, buf, strlen(buf), 0);
                //close(rsock);//the server close, so client can't recv, RST是向CLOSE-WAIT/TIMED_WAIT发包收到的把
                if(speaktimes %10000 ==0)
                    printf("[%s] [thread_request] [message] server->client len is %d, mesg is: %s\n", transtime(tv), strlen(buf), buf);
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
