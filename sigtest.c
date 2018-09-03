#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void handle_sig(int sig) {
    printf("signal number: %d\n", sig);
    switch (sig) {
        case SIGINT:
            printf("get signal: SIGINT\n");
            exit(0);
            break;
        case SIGTERM:
            printf("get signal: SIGTERM\n");
            exit(0);
            break;
        default:;
    }
    
}

int main(int argc , char *argv[]) {

    //signal(SIGINT, SIG_IGN);
    signal(SIGINT, handle_sig);
    signal(SIGTERM, handle_sig);

    printf("pid: %d\n",getpid());

    while (1) {
        printf("signal test\n");
        sleep(1);
    }

    return 0;
}

