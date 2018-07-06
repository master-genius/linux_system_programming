#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int _child_pid = 0;

void kill_child(int sig) {
    printf("time out\n");
    if (_child_pid > 0) {
        kill(_child_pid, SIGTERM);
    }
    kill(getpid(), SIGTERM);
}

void term_handle(int sig) {
    printf("get signal : sigterm -> %d\n",sig);
}

int main(int argc, char *argv[])
{
    
    signal(SIGALRM, kill_child);
    alarm(2);
    int pid = fork();
    if (pid<0){
        perror("fork");
        return -1;
    }
    if (pid > 0) {
        printf("start sleep...\n");
        _child_pid = pid;
        sleep(10);
    } else {
        signal(SIGTERM, term_handle);
        printf("child %d sleep...\n",getpid());
        sleep(8);
    }

    return 0;
}

