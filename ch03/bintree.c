#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void out_tree(int deep, int move) {
    if (deep <= 0)return;

    char fmt_str[16] = {'\0'};

    sprintf(fmt_str, "%%-%ds", move);
    //printf("|");
    printf(fmt_str,"");
    printf("|---T\n");
    
    out_tree(deep-1, move+4);
    out_tree(deep-1, move+4);

}

int main(int argc, char *argv[]) {

    out_tree(4, 0);

    return 0;
}
