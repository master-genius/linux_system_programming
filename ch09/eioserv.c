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
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>

#define DEF_PORT    2018

#define MAX_EVENTS  1024        //接收的最大请求

#define BUFF_SIZE   1001        

#define MAX_MSG_SIZE    8192    //单个消息最大字节数

/*
    消息状态 
*/
#define RECV_MSG_NONE   0
#define RECV_MSG_ERR    1
#define RECV_MSG_OK     2
#define RECV_MSG_CLOSE  3

/* 标记父进程和子进程 */
#define PCS_CHILD   1
#define PCS_PARENT  2

#define CHILD_COUNT     2

#define ACCEPT_MAX_LIMIT    MAX_EVENTS/CHILD_COUNT


union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

key_t _save_semid = 0;      //保存信号量ID

pid_t _self_pid;
int _accpet_count;  //accpet计数
int __child_count = 0;

void parent_handle_sig(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        printf("remove semid\n");
        semctl(_save_semid, 0, IPC_RMID);
        kill(0, SIGTERM);
        usleep(50000);
        exit(0);
    } else if (sig == SIGCHLD) {
        int st;
        pid_t pid;

        while((pid=waitpid(0, &st, WNOHANG)) > 0) {
            if (WIFEXITED(st) || WIFSIGNALED(st)) {
                printf("child %d exited\n", pid);
                __child_count -= 1;
            }
        }
    }
}

void child_handle_sig(int sig) {
    if (sig == SIGTERM || sig == SIGINT) {
        exit(0);
    }
}

int try_accpet_lock() {
    struct sembuf mf;
    mf.sem_num = 0;
    mf.sem_op = -1;
    mf.sem_flg = IPC_NOWAIT | SEM_UNDO;
    return semop(_save_semid, &mf, 1);
}

int release_accpet_lock() {
    struct sembuf mf;
    mf.sem_num = 0;
    mf.sem_op = 1;
    mf.sem_flg = IPC_NOWAIT | SEM_UNDO;
    return semop(_save_semid, &mf, 1);
}


struct msg_block {
    int msg_end;
    char msg[MAX_MSG_SIZE+1];
};

struct event_msg {
    int inuse;
    int fd;
    struct msg_block data;
};

//全局信息存储
struct event_msg _evtmsg[ACCEPT_MAX_LIMIT];




//设置套接字的文件描述符为非阻塞模式
int set_nonblocking(int fd) {
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    
    fcntl(fd, F_SETFL, new_opt);
    
    return old_opt;
}

//新创建的套接字连接加入到epoll列表中
void ep_addfd(int efd, int fd, int et){
    struct epoll_event evt;
    evt.data.fd = fd;
    evt.events = EPOLLIN;
    if (et)
        evt.events |= EPOLLET;
    epoll_ctl(efd, EPOLL_CTL_ADD, fd, &evt);

    set_nonblocking(fd);
}

//从epoll连接列表中删除套接字
void ep_delfd(int efd, int fd) {
    struct epoll_event evt;
    epoll_ctl(efd, EPOLL_CTL_DEL, fd, &evt);
}

//设置套接字的触发模式
void ep_modfd(int efd, int fd, int opt, int et) {
    struct epoll_event evt;
    evt.data.fd = fd;
    evt.events = opt;
    if (et)
        evt.events |= EPOLLET;
    epoll_ctl(efd, EPOLL_CTL_MOD, fd, &evt);
}



void save_msg(int fd, struct msg_block * msg) {
    int mi = fd % ACCEPT_MAX_LIMIT;
    int fi = -1;

    if ( (_evtmsg[mi].inuse && _evtmsg[mi].fd == fd)
        || 
        _evtmsg[mi].inuse==0
    ) {
        fi = mi;
    } else {
        for (int i=0; i<ACCEPT_MAX_LIMIT; i++) {
            if (_evtmsg[i].inuse==0) {
                fi = i;
                break;
            }
        }
    }
    if (fi>=0 && fi<ACCEPT_MAX_LIMIT) {
        _evtmsg[fi].inuse = 1;
        _evtmsg[fi].fd = fd;
        strcpy(_evtmsg[mi].data.msg, msg->msg);
    }
}

struct event_msg * get_msg(int fd) {
    for(int i=0;i<ACCEPT_MAX_LIMIT;i++) {
        if (_evtmsg[i].fd == fd) {
            return (_evtmsg+i);
        }
    }
    return NULL;
}

void close_socket(int fd) {
    close(fd);
    _accpet_count -= 1;
}

void event_et(struct epoll_event *evt, int number, int efd, int lisd) {
    int connfd = 0;
    struct sockaddr_in conn;
    socklen_t conn_size = sizeof(conn);

    char buf[BUFF_SIZE];
    int recv_count = 0;
    int recv_flag = RECV_MSG_NONE;

    for (int i=0; i<number; i++) {
        if (lisd == evt[i].data.fd) {
            if (_accpet_count >= ACCEPT_MAX_LIMIT)
                return;
            if (try_accpet_lock() < 0) {
                printf("%u : failed to get lock\n", getpid());
                return ;
            }

            connfd = accept(lisd, (struct sockaddr*)&conn, &conn_size);
            
            release_accpet_lock();

            if (connfd < 0) {
                //perror("accept");
                return ;
            }
            
            _accpet_count += 1;

            printf("%d : get connection...\n", _self_pid);
            ep_addfd(efd, connfd, 1);
        } else if (evt[i].events & EPOLLIN){
            printf("pid:%d get data from %d:\n", _self_pid, evt[i].data.fd);
            int msg_count = 0;
            struct msg_block msg;
            msg.msg_end = 0;
            msg.msg[0] = '\0';
            recv_flag = RECV_MSG_NONE;
            while (1) {
                memset(buf, '\0', BUFF_SIZE);

                recv_count = recv(evt[i].data.fd, buf, BUFF_SIZE-1, 0);
                if (recv_count < 0) {
                    if (errno==EAGAIN || errno==EWOULDBLOCK) {
                        recv_flag = RECV_MSG_OK;
                        break;
                    }
                    recv_flag = RECV_MSG_ERR;
                    close_socket(evt[i].data.fd);
                    break;
                } else if (recv_count == 0) { //connection closed
                    recv_flag = RECV_MSG_CLOSE;
                    ep_delfd(efd, evt[i].data.fd);
                    close_socket(evt[i].data.fd);
                    break;
                } else {
                    msg_count += recv_count;
                    if (msg_count < MAX_MSG_SIZE) {
                        recv_flag = RECV_MSG_OK;
                        strcat(msg.msg, buf);
                    }
                }
            }
            if (recv_flag == RECV_MSG_OK) {
                printf("bytes:%ld recieved: %s\n", strlen(msg.msg),msg.msg);

                if (strncmp(msg.msg, "//quit", 6)==0) {
                    ep_delfd(efd, evt[i].data.fd);
                    close_socket(evt[i].data.fd);
                    return ;
                }
                save_msg(evt[i].data.fd, &msg);
                ep_modfd(efd, evt[i].data.fd, EPOLLOUT, 1);
            } else if (recv_flag == RECV_MSG_CLOSE) {
                printf("client closed\n");
            } else {
                printf("Error: recv msg code = %d\n", recv_flag);
            }
        }else if (evt[i].events & EPOLLOUT){
            struct event_msg * emsg = get_msg(evt[i].data.fd);
            if (emsg == NULL)
                return ;
            printf("send msg: %s\n", emsg->data.msg);
            send(evt[i].data.fd, (void*)emsg->data.msg, strlen(emsg->data.msg), 0);
            ep_modfd(efd, evt[i].data.fd, EPOLLIN, 1);
        
        }else {
            printf("nothing to do\n");
        }
    }
}


int main(int argc, char * argv[]) {

    //start to daemon
    
    pid_t dpid = fork();
    if (dpid<0) {
        perror("fork");
        return 1;
    }
    if (dpid > 0) {
        exit(0);
    }
    setsid();

    //end daemon

    int process_flag = PCS_PARENT;

    //start to set semaphore
    key_t semk = 1024;
    int semid;
    semid = semget(semk, 1, IPC_CREAT|0666);
    if ( semid < 0 ) {
        perror("semget");
        return 1;
    }
    _save_semid = semid;

    union semun semarg;

    struct sembuf mf;
    mf.sem_num = 0;
    mf.sem_op = 1;
    mf.sem_flg = SEM_UNDO;

    semarg.val = 1;
    semarg.buf = NULL;
    semarg.array = NULL;
    semarg.__buf = NULL;

    if (semctl(semid, 0, SETVAL, semarg)<0) {
        semctl(semid, 0, IPC_RMID);
        perror("semctl");
        return 2;
    }

    semop(semid, &mf, 1);

    //end sem


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
    if(listen(lisd,13))
    {
        perror("listen");
        return -1;
    }

    pid_t pid;
    for(int i=0; i<CHILD_COUNT; i++) {
        pid = fork();
        if (pid < 0) {
            kill(0, SIGTERM);
            semctl(semid, 0, IPC_RMID);
            perror("fork");
            return 1;
        } else if (pid > 0) {
            __child_count += 1;
            continue;
        } else {
            process_flag = PCS_CHILD;
            break;
        }
    }


    if (process_flag == PCS_PARENT) {
        
        _self_pid = getpid();
        printf("daemon running:%u\n", _self_pid);

        signal(SIGINT, parent_handle_sig);
        signal(SIGTERM, parent_handle_sig);
        signal(SIGCHLD, parent_handle_sig);
        while (1) {
            sleep(2);
        }

    } else {
        _self_pid = getpid();
        signal(SIGINT, SIG_IGN);
        signal(SIGTERM, child_handle_sig);

        _accpet_count = 0;
        struct epoll_event events[ACCEPT_MAX_LIMIT];

        int efd = epoll_create1(0);
        if (efd < 0) {
            perror("epoll_create1");
            return 1;
        }

        ep_addfd(efd, lisd, 1);

        int event_count = 0;
        while(1) {
            event_count = epoll_wait(efd, events, ACCEPT_MAX_LIMIT, -1);
            if (event_count < 0) {
                perror("epoll_wait");
                break;
            }
            event_et(events, event_count, efd, lisd);
        }
        
        close(lisd);
        close(efd);
    }
    return 0;
}

