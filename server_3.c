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
#define CLIENTNUM 5
#define BACKLOG 10
#define MAXRECVLEN 24
#define MAXSERVERTIME 1024
void printids(const char *s);
static void *handle_request(void *argv);
static void handle_connect(int listenfd);
int connectfd_list[CLIENTNUM];
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
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    struct timeval tv;
    int connectfd;
    int i = 0;
    int idx[CLIENTNUM];
    for(i = 0; i < CLIENTNUM; i++){
        connectfd_list[i] = -1;
        idx[i] = i;
        pthread_create(&thread_do[i], NULL, handle_request, &idx[i]);
    }
    int server_times = 0;
    while(1){
        //stop server if the server has handled enough times
        if(server_times++ == MAXSERVERTIME)
            break;
        // connect here, then change the virable connectfd_list
        if((connectfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1){
            perror("accpet error.\n");
            exit(1);
        }
        gettimeofday(&tv, NULL);
        printf("You got a connection from client's ip %s, port %d at time %ld.%ld\n",inet_ntoa(client.sin_addr),htons(client.sin_port), tv.tv_sec,tv.tv_usec);
        for(i = 0;i<CLIENTNUM;i++){
            if(connectfd_list[i] == -1){
                connectfd_list[i] = connectfd;
                break;
            }    
        }
    }

    for(i = 0; i<CLIENTNUM;i++){
        pthread_join(thread_do[i]);    
    }
}

static void *handle_request(void *argv){
    int idx = *(int*)argv;
    printf("thread %d created, the connectfd is %d\n", idx, connectfd_list[idx]);
    char buf[MAXRECVLEN];
    while(1){
        int connectfd = connectfd_list[idx];
        // check if connect from virable connectfd_list...
        if (connectfd == -1){
            //printf("thread %d waitting...\n", idx);
            sleep(1);
            continue;
        }
        printf("thread %d to request\n", idx);
        
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
                connectfd_list[idx] = -1;
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
