#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>


int main(int argc, char *argv[]) {

    if (argc < 2) {
        dprintf(2, "error: usage killone [SIGNAL] [PID]\n");
        return -1;
    }

    int sig = SIGTERM;
    int pid = 0;
    for(int i=1; i<argc; i++) {
        if (strncmp(argv[i], "--signal=",9)==0) {
            sig = atoi(argv[i]+9);
            if (sig<0 || sig >64) {
                sig = SIGTERM;
            }
        } else {
            pid = atoi(argv[i]);
        }
    }

    if (pid == getpid() || pid <= 1) {
        dprintf(2, "Error: pid illagel\n");
        return -1;
    }

    if (kill(pid, sig)==-1) {
        perror("kill");
        return -1;
    }

    return 0;
}

