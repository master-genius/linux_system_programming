#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define DEF_PORT    2018

#define MAX_EVENTS  1024

#define BUFF_SIZE   1001

#define MAX_MSG_SIZE    8192

struct msg_block {
    int msg_end;
    char msg[MAX_MSG_SIZE+1];
};

struct event_msg {
    int inuse;
    int fd;
    struct msg_block data;
};

struct event_msg _evtmsg[MAX_EVENTS];

int set_nonblocking(int fd) {
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    
    fcntl(fd, F_SETFL, new_opt);
    
    return old_opt;
}

void ep_addfd(int efd, int fd, int et){
    struct epoll_event evt;
    evt.data.fd = fd;
    evt.events = EPOLLIN;
    if (et)
        evt.events |= EPOLLET;
    epoll_ctl(efd, EPOLL_CTL_ADD, fd, &evt);

    set_nonblocking(fd);
}

void ep_delfd(int efd, int fd) {
    struct epoll_event evt;
    epoll_ctl(efd, EPOLL_CTL_DEL, fd, &evt);
}

void ep_modfd(int efd, int fd, int opt, int et) {
    struct epoll_event evt;
    evt.data.fd = fd;
    evt.events = opt;
    if (et)
        evt.events |= EPOLLET;
    epoll_ctl(efd, EPOLL_CTL_MOD, fd, &evt);
}

void save_msg(int fd, struct msg_block * msg) {
    int mi = fd % MAX_EVENTS;
    int fi = -1;

    if ( (_evtmsg[mi].inuse && _evtmsg[mi].fd == fd)
        || 
        _evtmsg[mi].inuse==0
       )
    {
        fi = mi;
    } else {
        for (int i=0; i<MAX_EVENTS; i++) {
            if (_evtmsg[i].inuse==0) {
                fi = i;
                break;
            }
        }
    }
    if (fi>=0 && fi<MAX_EVENTS) {
        _evtmsg[fi].inuse = 1;
        _evtmsg[fi].fd = fd;
        strcpy(_evtmsg[mi].data.msg, msg->msg);
    }
}

struct event_msg * get_msg(int fd) {
    for(int i=0;i<MAX_EVENTS;i++) {
        if (_evtmsg[i].fd == fd) {
            return (_evtmsg+i);
        }
    }
    return NULL;
}

void event_et(struct epoll_event *evt, int number, int efd, int lisd) {
    int connfd = 0;
    struct sockaddr_in conn;
    socklen_t conn_size = sizeof(conn);

    char buf[BUFF_SIZE];
    int recv_count = 0;

    for (int i=0; i<number; i++) {
        if (lisd == evt[i].data.fd) {
            printf("get connection...\n");
            connfd = accept(lisd, (struct sockaddr*)&conn, &conn_size);
            ep_addfd(efd, connfd, 1);
        } else if (evt[i].events & EPOLLIN){
            printf("get data from %d:\n", evt[i].data.fd);
            int msg_count = 0;
            struct msg_block msg;
            msg.msg_end = 0;
            msg.msg[0] = '\0';

            while (1) {
                memset(buf, '\0', BUFF_SIZE);

                recv_count = recv(evt[i].data.fd, buf, BUFF_SIZE-1, 0);
                if (recv_count < 0) {
                    if (errno==EAGAIN || errno==EWOULDBLOCK) {
                        break;
                    }
                    close(evt[i].data.fd);
                    break;
                } else if (recv_count == 0) {
                    close(evt[i].data.fd);
                } else {
                    //printf("%s", buf);
                    msg_count += recv_count;
                    if (msg_count < MAX_MSG_SIZE)
                        strcat(msg.msg, buf);
                }
            }
            printf("recieved: %s\n", msg.msg);
            save_msg(evt[i].data.fd, &msg);
            ep_modfd(efd, evt[i].data.fd, EPOLLOUT, 1);
        }else if (evt[i].events & EPOLLOUT){
            struct event_msg * emsg = get_msg(evt[i].data.fd);
            if (emsg == NULL)
                return ;
            printf("send msg: %s\n", emsg->data.msg);
            send(evt[i].data.fd, (void*)emsg->data.msg, strlen(emsg->data.msg), 0);
            ep_modfd(efd, evt[i].data.fd, EPOLLIN, 1);
            ep_delfd(efd, evt[i].data.fd);
            close(evt[i].data.fd);
        
        }else {
            printf("nothing to do\n");
        }
    }
}

int main(int argc, char * argv[]) {

    struct sockaddr_in addr;

    int lisd = socket(PF_INET, SOCK_STREAM, 0);

    bzero((void*)&addr,sizeof(addr));

    if(lisd<0)
    {
        perror("socket");
        return -1;
    }

    int reuse = 1;
    setsockopt(lisd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    addr.sin_port=htons(DEF_PORT);
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    addr.sin_family=AF_INET;

    if(bind(lisd,(struct sockaddr*)&addr,sizeof(addr)))
    {
        perror("bind");
        return -1;
    }
    if(listen(lisd,12))
    {
        perror("listen");
        return -1;
    }

    struct epoll_event events[MAX_EVENTS];

    int efd = epoll_create1(0);
    if (efd < 0) {
        perror("epoll_create1");
        return 1;
    }

    ep_addfd(efd, lisd, 1);

    int event_count = 0;
    while(1) {
        event_count = epoll_wait(efd, events, MAX_EVENTS, -1);
        if (event_count < 0) {
            perror("epoll_wait");
            break;
        }
        event_et(events, event_count, efd, lisd);
    }
    
    close(lisd);
    close(efd);

    return 0;
}

