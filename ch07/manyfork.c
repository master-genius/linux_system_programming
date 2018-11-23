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
    if (process_flag == PCS_PARENT) {
        printf("parent:%d\n", getpid());
        int status = 0;
        for(int i=0;i<child; i++) {
            printf("child %d, exit code: %d\n",wait(&status), status);
        }
    } else if (process_flag == PCS_CHILD) {
        pid_t myid = getpid();
        printf("child:%d\n",myid);
        sleep(myid % 5);
    }

    return 0;
}

