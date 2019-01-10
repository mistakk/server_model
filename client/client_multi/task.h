#ifndef _TASK_H
#define _TASK_H
#include "public.h"

class CTask{
protected:
    string m_taskname;
    void* m_data;
public:
    CTask():m_taskname(""), m_data(NULL){};
    CTask(string &name):m_taskname(name), m_data(NULL){};
    virtual ~CTask(){};
    virtual bool Run(){printf("basic function run() called;\n"); return true;};
    void setdata(void *data){m_data = data;};
};

class mytask: public CTask{
public:
    bool Run(){
        pthread_t tid = pthread_self();
        printf("[message] [tid:%ld] data: %s;\n", tid, m_data);
        int x = rand()%4 + 1;
        sleep(x);
        return true;
    }
};

class clienttask: public CTask{
public:
    static int MAXDATASIZE;
    static int PORT; 
    bool Run();
};

#endif
