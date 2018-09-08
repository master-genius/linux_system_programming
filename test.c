#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <wchar_t.h>


int main(void) {

    char fmt_relt[512] = {'\0',};
    char fmtstr[128] = {'\0',};

    sprintf(fmtstr, "%%%ds", 64);

    sprintf(fmt_relt, fmtstr, "Linux is great\n");
    
    printf("%s", fmt_relt);

    return 0;
}

