//#include "tip.h"
#include "temp.h"
#include "threadpool.cpp"

void setNoBlock(int& fd);
void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);

int main(int argc, char *argv[])
{
    int listenfd, connfd, epfd, nfds;
    struct epoll_event ev, events[90];
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if(argc != 2){
        fprintf(stderr, "usage : %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    setNoBlock(listenfd);
    ev.data.fd = listenfd;
    ev.events = EPOLLIN
            |EPOLLET
            ;
    epfd=epoll_create(20);
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
    threadPoolInit();
    while(1){
        nfds = epoll_wait(epfd, events, 90, -1);
        for(int i=0; i<nfds; i++)
        {
            //如果是监听套接字，则建立连接
            if(events[i].data.fd == listenfd && ( EPOLLIN == ev.events & (EPOLLIN) ))
            {
                clientlen = sizeof(clientaddr);
                connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
                ev.data.fd = connfd;
                ev.events = EPOLLIN
                        |EPOLLET
                        |EPOLLONESHOT
                        ;
                setNoBlock(connfd);
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
            }
            //如果是链接套接字，则读取信息
            else
            {
//                Getnameinfo((SA*)&clientaddr, clientlen, hostname ,MAXLINE,
//                            port, MAXLINE, 0);
//                printf("Accepted connection from (%s, %s)\n", hostname, port);
                if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)
                    || (!(events[i].events & EPOLLIN))){
                    Close(events[i].data.fd);
                    continue;
                }
//                setNoBlock(events[i].data.fd);
                taskAdd(doit, events[i].data.fd);
//                Close(events[i].data.fd);
            }
        }
    }
    return 1;
}

void setNoBlock(int& fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flag);
}

//void doit(int fd)
//{
//    int is_static;
//    struct stat sbuf;
//    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
//    char filename[MAXLINE], cgiargs[MAXLINE];
//    rio_t rio;
//    cout<<23333;
//    Rio_readinitb(&rio, fd);
//    cout<<1111<<endl;
//    if (!Rio_readlineb(&rio, buf, MAXLINE))
//        return;

//    cout<<23333;
//    printf("%s", buf);
//    //第一行包括方法GET，uri，和HTTP版本
//    sscanf(buf, "%s %s %s", method, uri, version);
//    printf("%s,mmmm",method);printf("%s,uuuuu",uri);printf("%s,vvvvv",version);
//    if (strcasecmp(method, "GET")) {
//        clienterror(fd, method, "501", "Not Implemented",
//                    "Tiny does not implement this method");
//        return;
//    }
//    //读出剩下所有的报文头部
//    read_requesthdrs(&rio);
//    //解析uri，是否静态or动态内容
//    is_static = parse_uri(uri, filename, cgiargs);
//    if (stat(filename, &sbuf) < 0){
//        clienterror(fd, filename, "404", "Not found",
//                    "Tiny couldn't find this file");
//        return;
//    }
//    //静态
//    if (is_static){
//        if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)){
//            clienterror(fd, filename, "403", "Forbidden",
//                        "Tiny couldn't read the file");
//            return;
//        }
//        serve_static(fd, filename, sbuf.st_size);
//    }
//    //动态
//    else{
//        if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)){
//            clienterror(fd, filename, "403", "Forbidden",
//                        "Tiny couldn't run the CGI program");
//            return;
//        }
//        serve_dynamic(fd, filename, cgiargs);
//    }
//}

//void read_requesthdrs(rio_t *rp)
//{
//    char buf[MAXLINE];

//    Rio_readlineb(rp, buf, MAXLINE);
//    printf("%s", buf);
//    while(strcmp(buf, "\r\n")){
//        Rio_readlineb(rp, buf, MAXLINE);
//        printf("%s", buf);
//    }
//    return;
//}

////解析uri
//int parse_uri(char *uri, char *filename, char *cgiargs)
//{
//    char *ptr;

//    if(!strstr(uri, "bin")){
//        strcpy(cgiargs, "");
//        strcpy(filename, ".");
//        strcat(filename, uri);
//        //静态：设置默认文件路径
//        if (uri[strlen(uri)-1] == '/')
//            strcat(filename, "demo.html");
//        return 1;
//    }
//    else{
//        ptr = index(uri, '?');
//        if(ptr){//提取参数
//            strcpy(cgiargs, ptr+1);
//            *ptr = '\0';
//        }
//        else
//            strcpy(cgiargs, "");
//        //动态文件路径
//        strcpy(filename, ".");
//        strcat(filename, uri);
//        return 0;
//    }
//}


//void serve_static(int fd,  char *filename, int filesize)
//{
//    int srcfd;
//    char *srcp, filetype[MAXLINE], buf[MAXBUF];
//    //buf：响应头
//    get_filetype(filename, filetype);
//    sprintf(buf, "HTTP/1.0 200 OK\r\n");
//    sprintf(buf, "%sServer: TKeed Web Server\r\n", buf);
//    sprintf(buf, "%sConnection: close\r\n", buf);
//    sprintf(buf, "%sContent-lengthfuck: %d\r\n", buf, filesize);
//    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
//    Rio_writen(fd, buf, strlen(buf));
//    printf("Response headers:\n");
//    printf("%s", buf);
//    //srcp：响应z正文
//    srcfd = Open(filename, O_RDONLY, 0);
//    srcp = (char *)Mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);//映射内存
//    Close(srcfd);
//    Rio_writen(fd, srcp, filesize);
//    Munmap(srcp, filesize);//取消映射
//}

//void get_filetype(char *filename, char *filetype)\
//{
//    if(strstr(filename, ".html"))
//        strcpy(filetype, "text/html");
//    else if(strstr(filename, ".gif"))
//        strcpy(filetype, "image/gif");
//    else if(strstr(filename, ".png"))
//        strcpy(filetype, "image/png");
//    else if(strstr(filename, ".jpg"))
//        strcpy(filetype, "image/jpeg");
//    else
//        strcpy(filetype, "text/plain");
//}

//void serve_dynamic(int fd, char *filename, char *cgiargs)
//{
//    char buf[MAXLINE], *emptylist[] = { NULL };

//    sprintf(buf, "HTTP/1.0 200 OK\r\n");
//    Rio_writen(fd, buf, strlen(buf));
//    sprintf(buf, "Server: TKeed Web Server\r\n");
//    Rio_writen(fd, buf, strlen(buf));

//    if(Fork() == 0){
//        setenv("QUERY_STRING", cgiargs, 1);
//        Dup2(fd, STDOUT_FILENO);
//        Execve(filename, emptylist, environ);
//    }
//    Wait(NULL);
//}

//void clienterror(int fd, char *cause, char *errnum,
//                 char *shortmsg, char *longmsg)
//{
//    char buf[MAXLINE], body[MAXBUF];

//    sprintf(body, "<html><title>TKeed Error</title>");
//    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
//    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
//    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
//    sprintf(body, "%s<hr><em>The TKeed Web server</em>\r\n", body);

//    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
//    Rio_writen(fd, buf, strlen(buf));
//    sprintf(buf, "Content-type: text/html\r\n");
//    Rio_writen(fd, buf, strlen(buf));
//    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
//    Rio_writen(fd, buf, strlen(buf));
//    Rio_writen(fd, body, strlen(body));
//}
void doit(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE))  //line:netp:doit:readrequest
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);       //line:netp:doit:parserequest
    if (strcasecmp(method, "GET")) {                     //line:netp:doit:beginrequesterr
        clienterror(fd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }                                                    //line:netp:doit:endrequesterr
    read_requesthdrs(&rio);                              //line:netp:doit:readrequesthdrs

    /* Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs);       //line:netp:doit:staticcheck
    if (stat(filename, &sbuf) < 0) {                     //line:netp:doit:beginnotfound
        clienterror(fd, filename, "404", "Not found",
                "Tiny couldn't find this file");
        return;
    }                                                    //line:netp:doit:endnotfound

    if (is_static) { /* Serve static content */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) { //line:netp:doit:readable
            clienterror(fd, filename, "403", "Forbidden",
                "Tiny couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);        //line:netp:doit:servestatic
    }
    else { /* Serve dynamic content */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) { //line:netp:doit:executable
            clienterror(fd, filename, "403", "Forbidden",
                "Tiny couldn't run the CGI program");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);            //line:netp:doit:servedynamic
    }
}
/* $end doit */

/*
 * read_requesthdrs - read HTTP request headers
 */
/* $begin read_requesthdrs */
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    while(strcmp(buf, "\r\n")) {          //line:netp:readhdrs:checkterm
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}
/* $end read_requesthdrs */

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
/* $begin parse_uri */
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;
    printf("uri>>>%s<<<\n",uri);
    if (!strstr(uri, "cgi-bin")) {  /* Static content */ //line:netp:parseuri:isstatic
        strcpy(cgiargs, "");                             //line:netp:parseuri:clearcgi
        strcpy(filename, ".");                           //line:netp:parseuri:beginconvert1
        strcat(filename, uri);                           //line:netp:parseuri:endconvert1
        if (uri[strlen(uri)-1] == '/')                   //line:netp:parseuri:slashcheck
            strcat(filename, "home.html");               //line:netp:parseuri:appenddefault
        printf("filename>>>%s<<<\n",filename);
        return 1;
    }
    else {  /* Dynamic content */                        //line:netp:parseuri:isdynamic
        ptr = index(uri, '?');                           //line:netp:parseuri:beginextract
        if (ptr) {
            strcpy(cgiargs, ptr+1);
            *ptr = '\0';
        }
        else
            strcpy(cgiargs, "");                         //line:netp:parseuri:endextract
        strcpy(filename, ".");                           //line:netp:parseuri:beginconvert2
        strcat(filename, uri);                           //line:netp:parseuri:endconvert2
        printf("filename>>>%s<<<\n",filename);
        return 0;
    }
}
/* $end parse_uri */

/*
 * serve_static - copy a file back to the client
 */
/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype);       //line:netp:servestatic:getfiletype
    sprintf(buf, "HTTP/1.0 200 OK\r\n");    //line:netp:servestatic:beginserve
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));       //line:netp:servestatic:endserve
    printf("Response headers:\n");
    printf("%s", buf);

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);    //line:netp:servestatic:open
    srcp = (char*)Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);//line:netp:servestatic:mmap
    Close(srcfd);                           //line:netp:servestatic:close
    Rio_writen(fd, srcp, filesize);         //line:netp:servestatic:write
    Munmap(srcp, filesize);                 //line:netp:servestatic:munmap
}

/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
    strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
    else
    strcpy(filetype, "text/plain");
}
/* $end serve_static */

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if (Fork() == 0) { /* Child */ //line:netp:servedynamic:fork
    /* Real server would set all CGI vars here */
        setenv("QUERY_STRING", cgiargs, 1); //line:netp:servedynamic:setenv
        Dup2(fd, STDOUT_FILENO);         /* Redirect stdout to client */ //line:netp:servedynamic:dup2
        Execve(filename, emptylist, environ); /* Run CGI program */ //line:netp:servedynamic:execve
    }
    Wait(NULL); /* Parent waits for and reaps child */ //line:netp:servedynamic:wait
}
/* $end serve_dynamic */

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum,
         char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}
/* $end clienterror */
