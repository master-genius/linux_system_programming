#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {

    if (argc > 1 && strcmp(argv[1], "-2"))
        goto d_loop;

    for(int i=0; i<1000000; i++) {
        printf("Linux is great!\n");
    }

    goto end_loop;
    
d_loop:;
    int j,k;
    for(j=0; j<1000; j++)
        for(k=0; k<1000; k++) {
            printf("Linux is great!\n");
        }
    
end_loop:;

	return 0;
}

