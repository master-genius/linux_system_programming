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
    int port=PORT;
    char*ad="127.0.0.1";
    if(ac>1)
        ad=av[1];
    if(ac>2)
        port=atoi(av[2]);

    struct sockaddr_in addr;

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    bzero((void*)&addr,sizeof(addr));

    if(sock_fd<0)
    {
        perror("socket");
        return -1;
    }
    
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=inet_addr(ad);
    addr.sin_family=AF_INET;
    if(connect(sock_fd,(struct sockaddr*)(&addr),sizeof addr))
    {
        perror("connect");
        return -1;
    }

    char buf[256];

    int count=0;

    if((count=recv(sock_fd,(void*)buf,256,0))==-1)
    {
        perror("recv");
        printf("errno code:%d\n",errno);
        return -1;
    }

    buf[count]=0;
    //shutdown(sock_fd,SHUT_RDWR);

    close(sock_fd);
    printf("%s",buf);

    return 0;
}
