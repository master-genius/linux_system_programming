#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define DATA_BUF_LEN    1024

int _fd;
char _buffer[DATA_BUF_LEN+1];

void handle_sig(int sig) {
    printf("get signal: %d\n", sig);
    sleep(2);
    printf("close file\n");
    close(_fd);
}

int main(int argc, char *argv[]) {

    sigset_t blocked;
    struct sigaction sighandle;
    
    sigemptyset(&blocked);

    sigaddset(&blocked, SIGQUIT);

    sighandle.sa_mask = blocked;

    sighandle.sa_handler = handle_sig;

    if (sigaction(SIGINT, &sighandle, NULL) == -1) {
        perror("sigaction");
        return -1;
    }

    _fd = open("sigdatatest", O_CREAT|O_RDWR, S_IWUSR|S_IRUSR);
    if (_fd<0) {
        perror("open");
        return -1;
    }

    while (1) {
        fgets(_buffer, DATA_BUF_LEN,stdin);
        printf("writing to file: %s", _buffer);
        write(_fd, _buffer, strlen(_buffer));
    }

    return 0;
}
