#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <error.h>
#include <errno.h>

#define DATA_BUF_LEN    2048

int main(int argc, char *argv[]) {

    int ret = 0;
    char buffer[DATA_BUF_LEN+1] = {'\0', };
    int fd_old_opt, fd_cur_opt;

    fd_old_opt = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, O_NONBLOCK);

    int count = 0;
    int out_flag = 0;
    while (1) {
        count = read(0, buffer, DATA_BUF_LEN);
        if (count < 0 && (errno==EAGAIN || errno==EWOULDBLOCK) ) {
            if (out_flag > 16) {
                printf("waiting..\n");
                out_flag = 0;
            }
            out_flag += 1;
        } else if (count > 0) {
            printf("%s\n",buffer);
        }

        usleep(80000);
    }

    return ret;
}

