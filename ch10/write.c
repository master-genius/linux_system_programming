#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DATA_BUF_LEN    2048

int main(int argc, char *argv[]) {

    int ret = 0;
    char buffer[DATA_BUF_LEN+1] = "#!/bin/bash\nlscpu\nuname -a\nexit 0\n";

    int fd = open("datatest", O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    int count = 0;

    count = write(fd, buffer, strlen(buffer)+10);
    if (count < 0) {
        perror("write");
        ret = 2;
    } else if(count == 0) {
        printf("Write nothing\n");
    } else {
        printf("--write: \n%s\n--%d bytes\n", buffer, count);
    }

    close(fd);

    return ret;
}

