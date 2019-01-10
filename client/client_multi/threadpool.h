#ifndef _THREAD_POOL
#define _THREAD_POOL
#include "public.h"
#include "task.h"
class threadpool{
protected:
    static pthread_cond_t m_pthreadCond;
    static pthread_mutex_t m_pthreadMutex;//lock for m_tasklist / m_joinlist
    static pthread_mutex_t m_countMutex;
    static vector<CTask*> m_tasklist;//to do list
    static vector<pthread_t> m_joinlist;//wait for join list
    pthread_t *pthread_id;
    static bool stopflag;
protected:
    static void* threadfunc(void *idx);
    static int movetoidle(pthread_t tid);
    static int movetobusy(pthread_t tid);
    int create();

public:
    static int m_threadnum;//pthread count will join
    static int m_jobcount;//total job count
    static int m_jobsuccess;//case success count
    static int m_jobfailed;//case failed count

    int m_jobnum;//total added job number
    int m_joinnum;//total joined job number

    threadpool(int threadnum);//construst
    int stopall();
    int addtask(CTask* p);
    int gettasksize(){return m_tasklist.size();};
};

#endif
