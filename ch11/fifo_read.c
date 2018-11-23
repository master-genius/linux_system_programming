#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char *argv[]) {

    char *fifoname = "/tmp/fifo-test.fo";

    int fd = open(fifoname, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    char buffer[1000];
    int count = 0;

    count = read(fd, buffer, 1000);

    printf("%s\n", buffer);

    close(fd);

	return 0;
}

