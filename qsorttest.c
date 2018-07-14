#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mastertool.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        dprintf(2, "Error: enter some string\n");
        return -1;
    }

    int N = argc - 1;
    char ** qs = (char**)malloc(sizeof(char *) * N);
    if (qs == NULL) {
        perror("malloc");
        return -1;
    }
    
    for(int i=0; i<N; i++)
        qs[i] = argv[i+1];

    vqsort(qs, sizeof(char*)*N, sizeof(char*), str_comp);

    for(int i=0; i<N; i++)
        printf("%s ", qs[i]);

    free(qs);qs = NULL;
    printf("\n");

    double dt[10] = {12.3,234.4,345.456,456.654,67.768,546.4,56745,456,677,1331.1};
    vqsort(dt, sizeof(double)*10, sizeof(double), dou_comp);
    for(int i=0; i<10;i++)
        printf("%g ", dt[i]);
    printf("\n");

    return 0;
}

