#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

int list_dir(char * path);

int main(int argc, char *argv[])
{
    if (argc<2) {
        list_dir(".");
    }

    for (int i=1; i<argc; i++) {
        if (access(argv[i], F_OK)==0) {
            list_dir(argv[i]);
        } else {
            dprintf(2, "Error: %s is not file or directory\n", argv[i]);
        }
    }

    return 0;
}

int list_dir(char * path) {
    struct stat st;
    char flag = '\0';
    DIR * d = NULL;
    struct dirent * rd = NULL;
    if (lstat(path, &st)<0) {
        perror("lstat");
        return -1;
    }
    if (S_ISDIR(st.st_mode)) {
        if ((d=opendir(path))==NULL) {
            perror("opendir");
            return -1;
        }
        printf("%s/:\n",path);
        while((rd=readdir(d))!=NULL) {
            printf("%s\n", rd->d_name);
        }
        closedir(d);
    } else {
        printf("%s\n", path);
    }
    return 0;
}

