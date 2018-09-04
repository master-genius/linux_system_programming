#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if(pid > 0) {
        printf("Parent: %d fork child [%d]\n", getpid(), pid);
        int st = 0;
        printf("child %d exit code: %d\n",wait(&st),st);
    } else {
        printf("Child: %d\n",getpid());
        sleep(2);
    }

    return 0;
}
