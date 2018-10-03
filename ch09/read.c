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
    char buffer[DATA_BUF_LEN+1] = {'\0', };

    int fd = open("datatest", O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    int count = 0;

    count = read(fd, buffer, DATA_BUF_LEN);
    if (count < 0) {
        perror("read");
        ret = 2;
    } else if(count == 0) {
        printf("[File empty]\n");
    } else {
        printf("--read: \n%s\n--\n", buffer);
    }

    close(fd);

    return ret;
}

