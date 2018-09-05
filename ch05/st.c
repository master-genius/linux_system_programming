#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

void out_st_info(char * path, struct stat * st) {
    printf("%s -> size: %lu bytes\n", path, st->st_size);
    printf("    i-node: %-9lu  hard link: %-2lu ", st->st_ino, st->st_nlink);

    char * ptype = "";
    switch (st->st_mode & S_IFMT) {
        case S_IFDIR:
            ptype = "dir";
            break;
        case S_IFIFO:
            ptype = "fifo";
            break;
        case S_IFBLK:
            ptype = "block device";
            break;
        case S_IFCHR:
            ptype = "char device";
            break;
        case S_IFSOCK:
            ptype = "sock";
            break;
        case S_IFREG:
            ptype = "regular";
            if (access(path, X_OK)==0) {
                ptype = "regular*";
            }
            break;
        case S_IFLNK:
            ptype = "link";
            break;
    };

    printf("%s\n",ptype);
}

int main(int argc, char *argv[])
{
    struct stat st;

    for (int i=1; i<argc; i++) {
        if (access(argv[i], F_OK)) {
            dprintf(2, "Error: %s is not exists\n", argv[i]);
            continue;
        }

        if (lstat(argv[i], &st)==0) {
            out_st_info(argv[i], &st);
        } else {
            perror("lstat");
        }
    }

    return 0;
}

