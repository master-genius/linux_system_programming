#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char *argv[])
{
    alarm(2);
    pause();
    printf("I am a programmer\n");

    return 0;
}

