#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc<2) {
        dprintf(2,"Error:less DIR_NAME\n");
        return -1;
    }
    int mode_flag = 0;
    int mode = 0755;
    int i;
    int mode_buf = 0;
    char tmp;
    for(i=1;i<argc;i++) {
        if (strncmp(argv[i],"--mode=",7)==0) {
            if (strlen(argv[i]+7)!=3) {
                dprintf(2, "Error: mode is wrong\n");
                return -1;
            }
            for (int k=0;k<3;k++) {
                tmp = argv[i][7+k];
                if (tmp < '0' || tmp > '7') {
                    dprintf(2, "Error: mode number must in [0,7]\n");
                    return -1;
                }
                mode_buf += (tmp-48)*(1<<(3*(2-k)));
            }
            mode_flag = i;
            break;
        }
    }

    if (mode_flag>0 && argc==2) {
        dprintf(2,"Error: less DIR_NAME\n");
        return -1;
    }

    if (mode_flag>0 && mode_buf > 0)
        mode = mode_buf;

    for (i=1;i<argc;i++) {
        if (i==mode_flag)continue;
        if (mkdir(argv[i],mode) < 0) {
            perror("mkdir");
            return -1;
        }
    }

    return 0;
}

