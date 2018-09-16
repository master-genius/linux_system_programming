#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void out_tree(int deep, int root, int height) {
    if (deep <= 0)return;

    char fmt_str[16] = {'\0'};
    int space_end = (1<<(deep+height-1)) - height;
    for (int i=0; i<space_end; i++)
        printf(" ");

    if (root)
        printf("|\n");
    else{
        int end = 1<<(height-1);
        for (int i=0; i<end; i++)
            printf("/\\");
        printf("\n");
    }
    
    out_tree(deep-1, 0, height+1);
}

int main(int argc, char *argv[]) {

    out_tree(4, 1, 0);

    return 0;
}
