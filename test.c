#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define A_X     12
#define A_Y     13

#define X_D_Y   A_Y/A_X

int main(void) {
    printf("%d\n", X_D_Y);

//    char fmt_relt[512] = {'\0',};
//    char fmtstr[128] = {'\0',};
//
//    sprintf(fmtstr, "%%%ds", 64);
//
//    sprintf(fmt_relt, fmtstr, "Linux is great\n");
//    
//    printf("%s", fmt_relt);

    return 0;
}

