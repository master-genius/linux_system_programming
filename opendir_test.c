#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

int main(int argc, char *argv[])
{
    if (argc<2) {
        dprintf(2,"Error:less DIR_NAME\n");
        return -1;
    }

    DIR * dr = opendir(argv[1]);

    if (dr==NULL) {
        perror("opendir");
        return -1;
    }
    
    struct dirent * r = NULL;
    while((r=readdir(dr))!=NULL) {
        printf("%s\n", r->d_name);
    }

    return 0;
}

