#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "ctty.h"

//获取终端的行列数
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

//重置终端属性
void tty_reset(){
    printf("\033c");
}

void clear() {
    printf("\x1b[2J\x1b[;H");
}

void move(int row, int column) {
    printf("\x1b[%d;%dH", row, column);
}

//移动光标到指定位置输出字符串
void mvoutstr(int row, int column, char *s) {
    move(row, column);
    printf("%s", s);
    fflush(stdout);
}

//移动光标到指定位置输出字符
void mvoutc(int row, int column, char c) {
    move(row, column);
    printf("%c", c);
    fflush(stdout);
}

//允许滚屏
void enable_scroll() {
    printf("\x1b[r");
    fflush(stdout);
}

//允许部分区域滚屏
void scroll_zone(int start, int end) {
    printf("\033[%d;%dr", start, end);
    fflush(stdout);
}

/*
    滚屏操作
*/
void scroll_down(int lines) {
    for(int i=0; i<lines; i++) {
        printf("\033D");
    }
    fflush(stdout);
}

void scroll_up(int lines) {
    for(int i=0; i<lines; i++) {
        printf("\x1bM");
    }
    fflush(stdout);
}

/*
 *  光标移动操作
*/

void cursor_left(int count) {
    printf("\e[%dD", count);
    fflush(stdout);
}

void cursor_right(int count) {
    printf("\e[%dC", count);
    fflush(stdout);
}

void cursor_up(int count) {
    printf("\e[%dA", count);
    fflush(stdout);
}

void cursor_down(int count) {
    printf("\e[%dB", count);
    fflush(stdout);
}


