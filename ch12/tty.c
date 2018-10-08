#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>

int * winsize(int *ws) {
    ws[0] = ws[1] = 0;

    struct winsize _wz;
    ioctl (1, TIOCGWINSZ, &_wz);

    if (_wz.ws_col > 0)
        ws[1] = _wz.ws_col;
    if (_wz.ws_row > 0)
        ws[0] = _wz.ws_row;

    return ws;
}

void clear() {
    printf("\x1b[2J\x1b[;H");
}

void move(int row, int column) {
    printf("\x1b[%d;%dH", row, column);
}

void mvout(int row, int column, char *s) {
    move(row, column);
    printf("%s", s);
    fflush(stdout);
}


int main(int argc, char *argv[]) {

    int ws[2];
    winsize(ws);

    printf("row:%d\ncolumn:%d\n", ws[0], ws[1]);
    sleep(1);
    clear();

    for (int i=0; i<ws[1]-1; i++) {

        clear();
        move(i%(ws[0]-1), i);
        printf("%d", i);
        fflush(stdout);

        usleep(300000);
    }

	return 0;
}

