#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void out_tree(int deep, int move, int wtab, int flag) {
    if (deep <= 0)return;

    char fmt_str[16] = {'\0'};
    
    int align = 0;
    if (flag) {
        for(int i=0; i<move; i++) {
            printf("|   ");
        }
    } else {
        for(int i=1; i<move; i++) {
            printf("|   ");
        }
        printf("    ");
    }


    printf("|--[ ]\n");
    
    out_tree(deep-1, move+1, 4, 1);
    out_tree(deep-1, move+1, 4, 0);

}

int main(int argc, char *argv[]) {

    out_tree(4, 0, 4, 1);

    return 0;
}
