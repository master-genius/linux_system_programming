#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("\e[r");
    
    for (int i=0; i<12; i++) {
        printf("\033D");
        fflush(stdout);
        usleep(300000);
        printf("--%d", i+1);
    }
    printf("done\n");
	return 0;
}

