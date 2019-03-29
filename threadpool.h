#ifndef _POOL__
#define _POOL__
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>

using namespace std;

const int THREADNUMS = 5;

struct Task
{
    void (*func)(int);
    int arg;
    struct Task* next;
} ;

struct ThreadPool
{
    bool shutdown;
    pthread_cond_t cond;
    pthread_mutex_t mtx;
    pthread_t threads[THREADNUMS];
    struct Task *head;
} ;

void* workThread(void* arg, ThreadPool* myThreadPool);
void threadPoolInit(ThreadPool *myThreadPool);
void threadPoolDestory(ThreadPool* myThreadPool);
void taskAdd(ThreadPool* myThreadPool, void (*func)(int), int arg);

#endif
