#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("\e[r");
    
    for (int i=1; i<=42; i++) {
        printf("\033D");
        fflush(stdout);
        usleep(100000);
        printf("--%d", i);
        if (i%12 == 0) {
            printf("\e[%dD", i*3);
        }
    }
    for(int i=1; i<=3; i++) {
        printf("\033M");
        fflush(stdout);
    }
    printf("done\n");
	return 0;
}

