#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    struct stat st;

    if (fstat(1, &st)<0) {
        perror("fstat");
        return 1;
    }
    
    if (S_ISREG(st.st_mode)) {
        printf("regular file\n");
    } else if (S_ISBLK(st.st_mode)) {
        printf("block device\n");
    } else if (S_ISFIFO(st.st_mode)) {
        printf("FIFO/pipe file\n");
    } else if (S_ISCHR(st.st_mode)) {
        printf("char device\n");
    }

    return 0;
}
