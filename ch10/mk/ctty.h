#ifndef MY_CTTY_H
#define MY_CTTY_H

//获取终端的行列数
int * winsize(int *ws);

//重置终端属性
void tty_reset();

void clear();

void move(int row, int column);

//移动光标到指定位置输出字符串
void mvoutstr(int row, int column, char *s);

//移动光标到指定位置输出字符
void mvoutc(int row, int column, char c);

//允许滚屏
void enable_scroll();

//允许部分区域滚屏
void scroll_zone(int start, int end);

/*
    滚屏操作
*/
void scroll_down(int lines);

void scroll_up(int lines);

/*
 *  光标移动操作
*/

void cursor_left(int count);

void cursor_right(int count);

void cursor_up(int count);

void cursor_down(int count);


#endif

