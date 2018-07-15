#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

char *_path[] = {
    "/bin",
    "/sbin",
    "/usr/bin",
    "/usr/sbin",
    "/usr/local/bin",
    "/usr/local/sbin",
    "\0"
};

int main(int argc, char * argv[])
{
    char * path = getenv("HOME");
    char home_bin[256] = {'\0'};
    strcpy(home_bin, path);
    strcat(home_bin, "/bin");
    
    struct stat st;

    if (access(home_bin,F_OK|R_OK|X_OK)==0
        && lstat(home_bin, &st)==0
        && S_ISDIR(st.st_mode)
    ) {
    } else {
        home_bin[0] = '\0';
    }

    int pid = 0;
    char cmd_buf[8192] = {'\0'};
    int count = 0;
    while (1) {
        count = read(0,cmd_buf,8191);
        if (count<0) {
            perror("read");
            continue;
        } else {
            cmd_buf[count-1] = '\0';
            printf("%s\n", cmd_buf);
            continue;
        }

        pid = fork();
        if (pid < 0) {
            perror("fork");
            return 2;
        }

        if (pid > 0) {
            int status = 0;
            wait(&status);
        }

        if (pid == 0) {
            
        }
    }


    return 0;
}

