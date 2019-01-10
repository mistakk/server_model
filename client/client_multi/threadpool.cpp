#include "threadpool.h"

vector<CTask*> threadpool::m_tasklist;
vector<pthread_t> threadpool::m_joinlist;
bool threadpool::stopflag = false;
pthread_mutex_t threadpool::m_pthreadMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t threadpool::m_countMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t threadpool::m_pthreadCond = PTHREAD_COND_INITIALIZER;
int threadpool::m_threadnum = 0;
int threadpool::m_jobcount = 0;
int threadpool::m_jobfailed = 0;
int threadpool::m_jobsuccess = 0;

threadpool::threadpool(int pthreadnum){
    m_threadnum = pthreadnum;
    m_jobnum = 0;
    m_joinnum = 0;
    create();
}
int threadpool::create(){
    pthread_id = new pthread_t[m_threadnum];
    for(int i = 0; i<m_threadnum; ++i)
        pthread_create(&pthread_id[i], NULL, threadfunc, &i);
    return 0;
}
void* threadpool::threadfunc(void *idx){
    pthread_t tid = pthread_self();
    //pthread_detach(tid);
    while(1){
        pthread_mutex_lock(&m_pthreadMutex);
        while(m_tasklist.size()==0 && !stopflag){
            pthread_cond_wait(&m_pthreadCond, &m_pthreadMutex);
        }
        if(stopflag){
            //pthread_mutex_unlock(&m_pthreadMutex);
            //pthread_mutex_lock(&m_pthreadMutex);
            m_joinlist.push_back(tid);
            pthread_mutex_unlock(&m_pthreadMutex);
            pthread_exit(NULL);
        }
        vector<CTask*>::iterator iter = m_tasklist.begin();
        CTask *task = *iter;
        if(iter != m_tasklist.end()){
            task = *iter;
            m_tasklist.erase(iter);
        }
        pthread_mutex_unlock(&m_pthreadMutex);
        bool result = task->Run();
        pthread_mutex_lock(&m_countMutex);
        m_jobfailed += !result;
        m_jobsuccess += result;
        pthread_mutex_unlock(&m_countMutex);
    }
    return (void*)0;
}
int threadpool::addtask(CTask *task){
    pthread_mutex_lock(&m_pthreadMutex); 
    m_tasklist.push_back(task);
    m_jobnum+=1;
    pthread_mutex_unlock(&m_pthreadMutex);
    pthread_cond_signal(&m_pthreadCond);
    return 0;
}
int threadpool::stopall(){
    if(stopflag)
        return -1;
    stopflag = true;
    printf("[message] stop all the process;\n");
    pthread_cond_broadcast(&m_pthreadCond);
    vector<pthread_t>::iterator iter;
    
    while(m_threadnum != m_joinnum){
        printf("The threadnum is %d, joined is %d;\n", m_threadnum, m_joinnum);
        pthread_mutex_lock(&m_pthreadMutex);
        m_joinnum += m_joinlist.size();
        for(iter = m_joinlist.begin(); iter!=m_joinlist.end(); ++iter)
            pthread_join(*iter, NULL);
        m_joinlist.clear();
        pthread_mutex_unlock(&m_pthreadMutex);
        sleep(2);
    }
    for(int i = 0; i<m_threadnum; ++i);
        //pthread_join(pthread_id[i], NULL);
    delete[] pthread_id;
    printf("[message] recycle all thread success;\n");
    return 0;
}
