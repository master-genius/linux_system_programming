#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *agrv[]) {

    int strend = 0;
    char src[1024] = "Linux,Unix,Windowx,FreeBSD;BSD4.4,";
    char * dem = ";,";
    strend = strlen(src);

    char * sub = NULL;
    
    printf("source string : %s\n",src);

    sub = strtok(src, dem);
    while (sub!=NULL) {
        printf("%s\n", sub);
        sub = strtok(NULL, dem);
    }
//    for(int i=0; i<strend; i++)
//        printf("%d ", src[i]);
//    printf("\n");

    return 0;
}
