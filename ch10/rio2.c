#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    int fd = -1;
    
    fd = open("ioout",O_CREAT|O_APPEND|O_RDWR, S_IRUSR|S_IWUSR);
    if (fd<0) {
        perror("open");
        return -1;
    }

    dup2(fd,1);
    close(fd);
    
    printf("IO redirect %d\n", fd);

    return 0;
}

