#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {

    int fd[2];

    if (pipe(fd) < 0) {
        perror("pipe");
        return -1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
    }

    char buffer[1000]   = {'\0'};
    //parent
    if (pid > 0) {
        printf("Parent: writing data to child\n");
        sleep(1);
        char *data = "Hello, child. I am parent.\n";
        write(fd[1], data, strlen(data));
        close(fd[0]);
        close(fd[1]);
    } else {
        //child
        int count = 0;
        int err = 0;
        printf("Child: waiting data...\n");
        count = read(fd[0], buffer, 100);
        if (count < 0) {
            perror("read");
            err = -1;
        } else {
            printf("GET MESAGE : %s", buffer);
        }

        close(fd[0]);
        close(fd[1]);

        return err;
    }

	return 0;
}

