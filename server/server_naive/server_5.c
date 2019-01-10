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
static void *handle_request();
static void *handle_connect(void *argv);
int connectfd_list[CLIENTNUM];
int selectcnt;
int main(int argc, char *argv[])
{
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

    printf("hello world\n");
    pthread_create(&thread_do[0], NULL, handle_connect, (void*)&listenfd);
    pthread_create(&thread_do[1], NULL, handle_request, NULL);

    int i;
    for(i = 0; i<2; i++){
        pthread_join(thread_do[i]);    
    }

    close(listenfd);
    return 0;
}

static void *handle_connect(void* argv){
    printids("I'm handler connection thread!\n");
    int listenfd = *((int*)argv);
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    struct timeval tv; 
    int i = 0;
    int server_times = 0;
    int connectfd;
    selectcnt = 0;
    for(i = 0;i<CLIENTNUM;i++)
        connectfd_list[i] = -1;

    while(1){
        if(server_times++ == MAXSERVERTIME)
            break;
        
        if((connectfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1){
            perror("accpet error.\n");
            exit(1);
        }
        gettimeofday(&tv, NULL);
        printf("You got a connection from client's ip %s, port %d at time %ld.%ld\n",inet_ntoa(client.sin_addr),htons(client.sin_port), tv.tv_sec,tv.tv_usec);
        for(i = 0;i<CLIENTNUM;i++){
            printf("connectfd_list[i] val is %d\n", connectfd_list[i]);
            if(connectfd_list[i] == -1){
                connectfd_list[i] = connectfd;
                selectcnt++;
                printf("add selectcnt val is %d\n", selectcnt);
                break;
            }    
        }
    }
}

static void *handle_request(){
    printids("I'm handler request thread!\n");
    char buf[MAXRECVLEN];
    
    struct timeval timeout;
    fd_set readfds;
    int connectfd;
    int maxfd;
    int retval;
    int i;


    while(1){
        printf("request thread waiting client...\n");
        FD_ZERO(&readfds);
        maxfd = 0;
        for(i = 0;i < selectcnt; i++){
            FD_SET(connectfd_list[i], &readfds);
            if(connectfd_list[i] > maxfd)
                maxfd = connectfd_list[i];
        }

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        retval = select(maxfd+1, &readfds, NULL, NULL, &timeout);

        if(retval < 0)
            perror("error\n");
        else if (retval ==0)
            continue;

        memset(buf, 0, MAXRECVLEN);

        for(i = 0;i < selectcnt; i++){
            connectfd = connectfd_list[i];
            if(FD_ISSET(connectfd, &readfds)){
                int iret = -1;
                while(1){
                    iret = recv(connectfd, buf, MAXRECVLEN, 0);
                    if(iret > 0){
                        printf("%s\n", buf);
                    }
                    else{
                        printf("connection over!\n");    
                        close(connectfd);
                        connectfd_list[i] = -1;
                        selectcnt--;
                        break;
                    }
                    send(connectfd, buf, iret, 0);
                }
            }
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
