#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    
    struct stat st;
    if (fstat(1, &st)==-1) {
        perror("fstat");
        return 1;
    }

    if (S_ISCHR(st.st_mode)) {
        printf("\e[1;35mLinux is great\n\e[0;m");
    } else if (
        S_ISFIFO(st.st_mode)
        || S_ISREG(st.st_mode)
    ) {
        printf("\e[1;35mLinux is great\n\e[0;m");
        //printf("Linux is great\n");
    }

}
