#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#define PORT    12345

#define HOST_LEN    256

int main(int ac, char* av[])
{
    char*ad="127.0.0.1";
    if(ac>1)
        ad=av[1];
    struct sockaddr_in addr;

    struct hostent* hp;

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    bzero((void*)&addr,sizeof(addr));

    if(sock_fd<0)
    {
        perror("socket");
        return -1;
    }

    char hostname[HOST_LEN];

    gethostname(hostname,HOST_LEN);

    //hp=gethostbyname(hostname);

    //bcopy((void*)(*(hp->h_addr_list)), (void*)&addr.sin_addr,hp->h_length);

    addr.sin_port=htons(PORT);
    addr.sin_addr.s_addr=inet_addr(ad);
    addr.sin_family=AF_INET;

    unsigned char*b=(char*)(&addr.sin_addr.s_addr);

    printf("%d %d.%d.%d.%d\n",PORT,*(b+0),*(b+1),*(b+2),*(b+3));


    if(bind(sock_fd,(struct sockaddr*)&addr,sizeof(addr)))
    {
        perror("bind");
        printf("errno code:%d\n",errno);
        return -1;
    }

    if(listen(sock_fd,100))
    {
        perror("listen");
        return -1;
    }

    int sock_id;
    struct sockaddr_in user;
    int pid=0;

    time_t tm;

    char*p=NULL;

    int ulen=sizeof(user);
    
    fork();
    fork();
    printf("pid:%d\n",getpid());

    while(1)
    {
        sock_id=accept(sock_fd,(struct sockaddr*)&user,(socklen_t*)&ulen);
        if(sock_id>0)
        {
            b=(char*)&user.sin_addr.s_addr;
            printf("%d: get call from %d.%d.%d.%d\n",getpid(),*(b),*(b+1),*(b+2),*(b+3));
        }
        else
            continue;
        /*pid=fork();
        if(pid>0)
        {
            close(sock_id);
        }
        else if(pid==0)
        {*/
            tm=time(0);
            p=ctime(&tm);
            if(send(sock_id,(void*)p,strlen(p),0)==-1)
            {
                perror("send");
                return -1;
            }
            printf("send success\n");
            close(sock_id);
        //}
    }

    return 0;
}
