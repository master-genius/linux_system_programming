#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>
#include <stddef.h>

int main(int argc, char *argv[]) {

    for(int i=1; i<argc; i++) {
        printf("%s : %lu\n", argv[i], wctype(argv[i]));
    }

    return 0;
}
