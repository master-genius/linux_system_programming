#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

int main (int argc, char *argv[])
{
    if (argc<2) {
        if(chdir(getenv("HOME"))<0){
            perror("chdir");
            return -1;
        }
    }
    else if (chdir(argv[1])<0) {
        perror("chdir");
        return -1;
    }
    char buf[512] = {'\0'};
    printf("%s\n",getcwd(buf,500));

    return 0;
}
