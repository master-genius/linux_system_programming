#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

int main(int argc, char *argv[]) {
    
    int pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid>0) {
        return 0;
    }

    chdir("/");
    setsid();
    
    close(0);
    close(1);
    close(2);

    int fd = open("/dev/null", O_RDWR);
    dup2(fd, 0);
    dup2(fd, 1);
    dup2(fd, 2);
    close(fd);

    fd = open("/tmp/dserv", O_CREAT|O_APPEND|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd<0) {
        return -1;
    }
    char log[1024] = {'\0'};
    char *tm = NULL;
    time_t t;
    int count = 0;
    while(1) {
        time(&t);
        tm = ctime(&t);
        strcpy(log, tm);
        write(fd, log, strlen(log));
        count++;
        sleep(2+count/3);
    }

    return 0;
}

