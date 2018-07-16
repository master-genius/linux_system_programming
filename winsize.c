#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>

int main(int argc, char * argv[])
{
    
    struct winsize wz;
    ioctl (1, TIOCGWINSZ, &wz);
    printf("%d %d\n", wz.ws_row, wz.ws_col);

    return 0;
}
