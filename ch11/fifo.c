#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char *argv[]) {

    char *fifoname = "/tmp/fifo-test.fo";

    int fifo = mkfifo(fifoname, 0644);

    if (fifo < 0) {
        perror("mkfifo");
        return -1;
    }

    int fd = open(fifoname, O_WRONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    char *data = "时间具有相对性";
    write(fd, data, strlen(data));

    close(fd);

	return 0;
}

