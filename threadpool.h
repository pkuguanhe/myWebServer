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
} *task;

struct ThreadPool
{
    bool shutdown;
    pthread_cond_t cond;
    pthread_mutex_t mtx;
    pthread_t threads[THREADNUMS];
    struct Task *head;
} *myThreadPool;
