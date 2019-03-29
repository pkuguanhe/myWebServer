
#include "gloable.h"
#include "threadpool.h"
#include "myhttp.h"

int main(int argc, char *argv[])
{
    int listenfd, connfd, epfd, nfds;
    struct epoll_event ev, events[90];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    struct ThreadPool *myThreadPool;

    if(argc != 2){
        fprintf(stderr, "usage : %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    setNoBlock(listenfd);
    ev.data.fd = listenfd;
    ev.events = EPOLLIN | EPOLLET;
    epfd=epoll_create(20);
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
    threadPoolInit(myThreadPool);
    while(1){
        nfds = epoll_wait(epfd, events, 90, -1);
        for(int i=0; i<nfds; i++)
        {
            //如果是监听套接字，则建立连接
            if(events[i].data.fd == listenfd)
            {
                clientlen = sizeof(clientaddr);
                connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
                ev.data.fd = connfd;
                ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                setNoBlock(connfd);
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
            }
            //如果是链接套接字，则读取信息
            else
            {
                if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)
                        || (!(events[i].events & EPOLLIN))){
                    Close(events[i].data.fd);
                    continue;
                }
                taskAdd(myThreadPool, doit, events[i].data.fd);
            }
        }
    }
    return 1;
}
