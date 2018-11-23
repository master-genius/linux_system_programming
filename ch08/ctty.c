#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    printf("\x1b[46m\x1b[30m");
    printf("Linux is great\n");
    printf("\x1b[0m");

	return 0;
}
