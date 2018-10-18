#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/input.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <signal.h>


#define DEVS_FILE   "/proc/bus/input/devices"

#define BUF_LEN     4096

#define DEV_ST_LINE 9

#define DEV_LINE_SIZE   512

char _dev_struct[DEV_ST_LINE][DEV_LINE_SIZE+1];
char _at_kbd_file[256];
char _usb_kbd_file[256];
int kfd[2] = {-1,-1};

void handle_sig(int sig) {
    switch(sig) {
        case SIGINT:
        case SIGTERM:
            for(int i=0; i<2; i++) {
                if (kfd[i] > 0)
                    close(kfd[i]);
            }
            exit(0);
        default:;
    }

}

int init_dev_file() {
    int errcode = 0;
    FILE * f = fopen(DEVS_FILE, "r");
    if (f == NULL){
        perror("fopen");
        return -1;
    }
    char *p = NULL;
    int i=0;
    int k = 0;
    int len  = 0;
    char *kp = NULL;
    char *ep;
    while (1) {
        p = fgets(_dev_struct[i], DEV_LINE_SIZE, f);
        if (errno && p == NULL) {
            errcode = -1;
            goto end_dev;
        }else if (p == NULL) {
            break;
        }
        if (strcmp(p, "\n") == 0) {
            i=0;
            continue;
        }

        i++;
        if (i < DEV_ST_LINE) {
            continue;
        } else {
            i = 0;
        }
        if (strstr(_dev_struct[1], "keyboard") || strstr(_dev_struct[1], "Keyboard")) {
            k = 0;
            len = strlen(_dev_struct[1]);
            while(k<len) {
                if (_dev_struct[1][k++] == '"')
                    break;
            }
            if (strncmp(_dev_struct[1]+k, "USB", 3) == 0) {
                kp = _usb_kbd_file;
            } else if (strncmp(_dev_struct[1]+k, "AT" , 2) == 0){
                kp = _at_kbd_file;
            }
            
            if (strstr(_dev_struct[2], "input0")) {
                ep = strstr(_dev_struct[5], "event");
                if (ep != NULL) {
                    for(int j=0;j<strlen(ep); j++) {
                        if (ep[j]==' ') {
                            ep[j] = '\0';
                            break;
                        }
                    }
                    strcpy(kp, "/dev/input/");
                    strcat(kp, ep);
                }

            }
        }

    }
    

end_dev:;
    fclose(f);

    return errcode;
}

int set_nonblocking(int fd) {
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    
    fcntl(fd, F_SETFL, new_opt);
    
    return old_opt;
}

void handle_key_evt(int fd) {

    struct input_event kt;
    int count = 0;

    count = read(fd, &kt, sizeof(kt));

    if (count < 0 && errno == EAGAIN) {
        return ;
    } else if (count > 0) {
        if (kt.type == EV_KEY) {
            if (kt.value == 0 || kt.value == 1) {
                printf("key %d %s\n", kt.code, kt.value?"pressed":"released");
                if (kt.code == KEY_ESC)
                    kill(getpid(), SIGTERM);
            } 
        }
    } else {
        perror("read");
    }
}


int main(int argc, char *argv[]) {

    signal(SIGINT, handle_sig);
    signal(SIGTERM, handle_sig);
   
    _at_kbd_file[0] = '\0';
    _usb_kbd_file[0] = '\0';

    if (init_dev_file() < 0) {
        return -1;
    }
    printf("%s\n%s\n", _at_kbd_file, _usb_kbd_file);

    int kfd_count = 0;

    if (strlen(_at_kbd_file) > 0) {
        kfd[0] = open(_at_kbd_file, O_RDONLY);
        if (kfd[0] < 0) {
            dprintf(2, "%s\n", _at_kbd_file);
        } else {
            kfd_count++;
        }
    }

    if (strlen(_usb_kbd_file) > 0) {
        kfd[1] = open(_usb_kbd_file, O_RDONLY);
        if (kfd[1] < 0) {
            dprintf(2, "%s\n", _usb_kbd_file);
            perror("open");
        }
        else {
            kfd_count++;
        }
    }

    if (kfd_count <= 0) {
        dprintf(2, "Error: failed to open input device file\n");
        return -1;
    }
    
    for(int i=0;i<2;i++) {
        if (kfd[i] > 0)
            set_nonblocking(kfd[i]);
    }

    while (1) {
        if (kfd[1] > 0) {
            handle_key_evt(kfd[1]);
        } 

        if (kfd[0] > 0) {
            handle_key_evt(kfd[0]);
        }

        //usleep(500000);
    }
    
	return 0;
}

