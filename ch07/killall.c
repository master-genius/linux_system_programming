#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <wait.h>

#define PCS_PARENT  1
#define PCS_CHILD   2
#define PCS_UNSET   0

int process_flag = PCS_UNSET;

void handle_sig(int sig) {
    printf("%d get signal: %u  ", getpid(), sig);
    switch (sig) {
        case SIGINT:
            printf("SIGINT -> exit...\n");
            exit(0);
        case SIGTERM:
            printf("SIGTERM -> exit...\n");
            exit(0);
        case SIGALRM:
            printf("@@\n");
            break;
        default:;
    }
}

int main(int argc, char *argv[]) {
    
    process_flag = PCS_PARENT;

    int child = 3;
    pid_t pid;

    for(int i=0; i<child; i++) {
        pid = fork();

        if (pid<0) {
            perror("fork");
            kill(0, SIGTERM);
            return -1;
        }

        if (pid>0) {
            continue;
        } else {
            process_flag = PCS_CHILD;
            break;
        }
    }
    if (process_flag == PCS_UNSET) {
    
    } else if (process_flag == PCS_PARENT) {
        printf("parent %d will kill all child\n ", getpid());
        sleep(5);
        kill(0, SIGTERM);
    } else if (process_flag == PCS_CHILD) {
        pid_t myid = getpid();
        signal(SIGTERM, handle_sig);
        while(1) {
            printf("child %d say : hello liux.\n", myid);
            sleep(1);
        }
    }

    return 0;
}

