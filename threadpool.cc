
#include "gloable.h"
#include "threadpool.h"
void* workThread(void* arg)
{
    ThreadPool* myThreadPool=(ThreadPool*)arg;
    pthread_detach(pthread_self());
    while(1)
    {
        if(myThreadPool->shutdown)
            break;
        pthread_mutex_lock(&(myThreadPool->mtx));

        pthread_cond_wait(&(myThreadPool->cond), &(myThreadPool->mtx));
        Task *newTask = myThreadPool->head->next;
        myThreadPool->head->next = myThreadPool->head->next->next;

        pthread_mutex_unlock(&(myThreadPool->mtx));
        (*(newTask->func))(newTask->arg);
        free(newTask);
    }
    cout<<"thread end"<<endl;
    pthread_exit(0);
}

void threadPoolInit(ThreadPool *myThreadPool)
{
    myThreadPool = (ThreadPool*)malloc(sizeof(ThreadPool));
    myThreadPool->shutdown = false;
    myThreadPool->head = (Task*)malloc(sizeof(Task));
    myThreadPool->head->func = NULL;
    myThreadPool->head->arg = -1;
    myThreadPool->head->next = NULL;

    pthread_mutex_init(&(myThreadPool->mtx), NULL);
    pthread_cond_init(&(myThreadPool->cond), NULL);
    for(int i=0; i<THREADNUMS; i++)
    {
        pthread_create(&(myThreadPool->threads[i]), NULL, workThread, NULL);
    }
}

void threadPoolDestory(ThreadPool* myThreadPool)
{
    myThreadPool->shutdown = true;
    return;
}

void taskAdd(ThreadPool* myThreadPool, void (*func)(int), int arg)
{
    Task* newTask = (Task*)malloc(sizeof(Task));
    newTask->func = func;
    newTask->arg = arg;

    pthread_mutex_lock(&(myThreadPool->mtx));

    newTask->next = myThreadPool->head->next;
    myThreadPool->head->next = newTask;

    pthread_cond_signal(&(myThreadPool->cond));

    pthread_mutex_unlock(&(myThreadPool->mtx));
}
