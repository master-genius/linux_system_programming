#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

void handle_sig(int sig) {
    printf("get signal: %d\n",sig);
    exit(0);
}

int main(int argc, char *argv[]) {
    
    signal(SIGTERM, handle_sig);

    /*
    while (1) {
        printf("waiting signal...\n");
        sleep(1);
        kill(getpid(), SIGTERM);
    }*/

    printf("waiting signal...\n");

    sleep(1);

    kill(getpid(), SIGTERM);

    sleep(5);

    return 0;
}

