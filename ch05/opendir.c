#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    if (argc<2) {
        dprintf(2,"Error:less DIR_NAME\n");
        return -1;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }
    printf("open:%d,starting read data\n",fd);
    
    int count = 0, k=0;
    char buf[2048] = {'\0'};
    while((count=read(fd,buf,2000))>0) {
        k=0;
        while(k<count) {
            printf("%c",buf[k++]);
        }
    }

    return 0;
}

