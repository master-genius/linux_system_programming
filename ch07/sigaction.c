#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

#define DATA_BUF_LEN    1024

char _buffer[DATA_BUF_LEN+1];

void handle_sig(int sig) {
    printf("get signal: %d\n", sig);
    sleep(1);
    time_t tm = time(NULL);
    printf("done : %s", ctime(&tm));
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

    while (1) {
        fgets(_buffer, DATA_BUF_LEN,stdin);
        printf("    %s", _buffer);
    }

    return 0;
}
