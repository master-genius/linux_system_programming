#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char *argv[])
{
    int sd = 0;
    sd = alarm(2);
    sd = alarm(0);
    printf("%u \n", sd);
    pause();
    printf("I am a programmer\n");

    return 0;
}

