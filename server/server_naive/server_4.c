#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 2222
#define CLIENTNUM 2
#define BACKLOG 1
#define MAXRECVLEN 24
void printids(const char *s);
static void *handle_request(void *argv);
static void handle_connect(int listenfd);
pthread_mutex_t ALOCK;
int main(int argc, char *argv[])
{
    int listenfd, connectfd;   /* socket descriptors */
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
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

    handle_connect(listenfd);
    close(listenfd);
    return 0;
}

static void handle_connect(int listenfd){
    pthread_t thread_do[CLIENTNUM];
    int i = 0;
    for(i = 0; i < CLIENTNUM; i++){
        pthread_create(&thread_do[i], NULL, handle_request, (void*)&listenfd);
    }
    for(i = 0; i<CLIENTNUM;i++){
        pthread_join(thread_do[i]);    
    }
}

static void *handle_request(void *argv){
    int listenfd = *((int*)argv);
    int connectfd;
    char buf[MAXRECVLEN];
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    while(1){
        pthread_mutex_lock(&ALOCK);
        if((connectfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1){
            perror("accpet error.\n");
            exit(1);
        }
        printids("luckone");
        pthread_mutex_unlock(&ALOCK);
        struct timeval tv;
        gettimeofday(&tv, NULL);
        printf("You got a connection from client's ip %s, port %d at time %ld.%ld\n",inet_ntoa(client.sin_addr),htons(client.sin_port), tv.tv_sec,tv.tv_usec);
        
        int iret=-1;
        while(1)
        {
            iret = recv(connectfd, buf, MAXRECVLEN, 0);
            if(iret>0)
            {
                printf("%s\n", buf);
            }else
            {
                printf("ready to exit()\n");
                close(connectfd);
                break;
            }
            /* print client's ip and port */
            send(connectfd, buf, iret, 0); /* send to the client welcome message */
        }
    }
    return NULL;
}

void printids(const char *s){
    pid_t pid;
    pthread_t tid;
    
    pid = getpid();
    tid = pthread_self();
    printf("%s pid %lu tid %lu (0x%lx)\n", s, (unsigned long)pid, (unsigned long)tid, (unsigned long)tid);    
    
}
