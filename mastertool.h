#ifndef MASTERTOOL_H
#define MASTERTOOL_H



#include <string.h>

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

int dou_comp(const void *a, const void *b) {
    const double * x = a;
    const double * y = b;
    return (*x ==*y)?0:((*x > *y)?1:-1);
}

void int_qsort(int* nms, int n) {
    vqsort(nms, sizeof(int)*n, sizeof(int), int_comp);
}

void dou_qsort(double * ds, int n) {
    vqsort(ds, sizeof(double)*n, sizeof(double), dou_comp);
}

void str_qsort(char* str[], int n) {
    vqsort(str, sizeof(char*)*n, sizeof(char*), str_comp);
}

#endif

