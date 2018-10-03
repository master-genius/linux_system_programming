#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {

    int fd = open("dataopen", O_CREAT|O_RDWR, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    close(fd);

    return 0;
}

