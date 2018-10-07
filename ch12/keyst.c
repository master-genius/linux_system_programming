#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/input.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>


#define KEY_EVENT_FILE  "/dev/input/event2"

int set_nonblocking(int fd) {
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    
    fcntl(fd, F_SETFL, new_opt);
    
    return old_opt;
}


int main(int argc, char *argv[]) {

    int kfd = open(KEY_EVENT_FILE, O_RDONLY);
    if (kfd < 0) {
        perror("open");
        return -1;
    }

    set_nonblocking(kfd);

    struct input_event kt;
    
    int count = 0;

    while (1) { 
        count = read(kfd, &kt, sizeof(kt));

        if (count < 0 && errno == EAGAIN) {
            //printf("shit\n");
            continue;
        } else if (count > 0) {
            if (kt.type == EV_KEY) {
                if (kt.value == 0 || kt.value == 1) {
                    printf("key %d %s\n", kt.code, kt.value?"pressed":"released");
                    if (kt.code == KEY_ESC)
                        break;
                } else {
                    printf("fuck\n");
                }
            }
        } else {
            perror("read");
        }
        //usleep(500000);
    }
    
    close(kfd);

	return 0;
}

