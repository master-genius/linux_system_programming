#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void handle_sig(int sig) {
    int st;
    pid_t pid;

    while((pid=waitpid(0, &st, WNOHANG)) > 0) {
        if (WIFEXITED(st)) {
            printf("child %d exited\n", pid);
        } else if (WIFSIGNALED(st)) {
            printf("child %d terminate by signal\n", pid);
        }
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if(pid > 0) {
        printf("Parent: %d fork child [%d]\n", getpid(), pid);
        signal(SIGCHLD, handle_sigchld);
        while (1) {
            printf("waiting child exit...\n");
            sleep(1);
        }
    } else {
        printf("Child: %d\n",getpid());
        sleep(5);
    }

    return 0;
}
