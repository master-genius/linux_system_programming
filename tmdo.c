#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void alarm_do(int sig) {
    //signal(SIGALRM, alarm_do);
    printf("hello\n");
}

int main(int argc, char *argv[]) {

    signal(SIGALRM, alarm_do);
    while(1) {
        alarm(1);
        pause();
    }

    return 0;
}
