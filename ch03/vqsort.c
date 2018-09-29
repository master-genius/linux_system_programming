#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SWAP(a,b)   tmp=a;a=b;b=tmp;

void qsort_core(void * base, int start, int end,
    unsigned int size, int (*comp)(const void *, const void *)
);

void vqsort(void* base, unsigned int nmemb, unsigned int size, 
    int(*comp)( const void *, const void *)
) {
    qsort_core(base, 0, nmemb/size - 1, size, comp);
}

int int_comp(const void *a, const void *b) {
    int const * x = a;
    int const * y = b;
    return (*x == *y)?0:((*x > *y)?1:-1);
}

int str_comp(const void *a, const void *b) {
    return strcmp(*(char**)a, *(char**)b);
}

int d_comp(const void *a, const void *b) {
    const double * x = a;
    const double * y = b;
    return (*x ==*y)?0:((*x > *y)?1:-1);
}


int main(int argc, char *argv[])
{
    if (argc < 2) {
        goto double_sort;
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

double_sort:;
    double dt[10] = {12.3,234.4,345.456,456.654,67.768,546.4,56745,456,677,1331.1};
    vqsort(dt, sizeof(double)*10, sizeof(double), d_comp);
    for(int i=0; i<10;i++)
        printf("%g ", dt[i]);
    printf("\n");


    return 0;
}

void qsort_core(void * base, int start, int end,
    unsigned int size, int (*comp)(const void *, const void *)
) {
    if (start >= end) {
        return ;
    }
    
    int med = (end+start)/2;
    int k = start;
    int j;
    char tmp;
    char * b = base;

    for (int i=0;i<size;i++) {
        SWAP(b[med*size+i],b[start*size+i]);
    }

    for(j=start+1;j<=end;j++) {
        if (comp(b+j*size,b+start*size) < 0) {
            k += 1;
            for(int i=0;i<size;i++) {
                SWAP(b[k*size+i],b[j*size+i]);
            }
        }
    }

    for (int i=0; i<size; i++) {
        SWAP(b[k*size+i],b[start*size+i]);
    }

    qsort_core(base, start, k-1, size, comp);
    qsort_core(base, k+1, end, size, comp);
}

