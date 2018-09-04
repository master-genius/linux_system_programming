#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    if ( seteuid(0) ) {
        perror("seteuid");
        return 1;
    }

    printf("uid: %u\n",getuid());

    return 0;
}
