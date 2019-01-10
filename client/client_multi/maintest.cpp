#include "threadpool.h"
#include "task.h"
int main(){
    int pthreadnum = 3000;
    int testnum = 10000, failedcnt;
    threadpool th(pthreadnum);
    clienttask p2 = clienttask();
    struct timeval tv, tv_first;
    for(int i = 0; i<testnum; ++i){
        th.addtask(&p2);
    }
    gettimeofday(&tv_first, NULL);

    int staytime = 0;
    int lastcnt = 0;
    while(1) {
        printf("There are still %d tasks need to handle\n", th.gettasksize());
        printf("There are %d thread, test %d times, failed %d times;\n", pthreadnum, th.m_jobfailed + th.m_jobsuccess, th.m_jobfailed);
        //任务队列已没有任务了
        if(th.m_jobfailed + th.m_jobsuccess == testnum) {
            //清除线程池
            failedcnt = th.m_jobfailed;
            if(th.stopall() == -1) {
                printf("Thread pool clear, exit.\n");
                break;
            }
        }
        if(th.m_jobfailed + th.m_jobsuccess == lastcnt)
            staytime ++;
        if(staytime ==3)
            th.stopall();
        lastcnt = th.m_jobfailed + th.m_jobsuccess;
        sleep(2);
        printf("2 seconds later...\n");
    }   
    gettimeofday(&tv, NULL);
    int milliseconds = (tv.tv_sec - tv_first.tv_sec);
    printf("There are %d thread, test %d times, failed %d times, cost %d seconds;\n", pthreadnum, testnum, failedcnt, milliseconds);

    return 0;
}
