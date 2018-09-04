#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if(pid > 0) {
        printf("Parent: %d fork child [%d]\n", getpid(), pid);
    } else {
        printf("Child: %d\n",getpid());
    }

    return 0;
}
