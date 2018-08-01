#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

    for (int i=1; i<argc; i++) {
        printf("%f %d\n", strtod(argv[i], NULL), strtol(argv[i], NULL, 10));
        //printf ("%f %d \n", atof(argv[i]), atoi(argv[i]));
    }

    printf("\n");

    return 0;
}
