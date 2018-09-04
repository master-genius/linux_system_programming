#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

void alarm_do(int sig) {
    time_t tm = time(NULL);
    printf("%s",ctime(&tm));
}

int main(int argc, char *argv[]) {

    signal(SIGALRM, alarm_do);
    while(1) {
        alarm(1);
        pause();
    }

    return 0;
}
