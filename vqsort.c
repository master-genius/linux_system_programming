#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//void qsort(void *d, unsigned int size, int (*call)(void * a, void *b));

#define SWAP(a,b)   tmp=a;a=b;b=tmp;

void qsort_core(void * base, unsigned int start, unsigned int end,
    unsigned int size, int (*comp)(const void *, const void *)
);

void qsort(void* base, unsigned int nmemb, unsigned int size, 
    int(*comp)( const void *, const void *)
) {
    qsort_core(void* base, 0, nmemb/size, size, comp);
}





int main(int argc, char *argv[])
{
    if (argc < 2) {
        dprintf(2, "Error: enter some string\n");
        return -1;
    }

    int N = argc - 1;

    qsortstr(argv, 1, N);

    for (int i=1;i<=N;i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");

    return 0;
}

void qsort_core(void * base, unsigned int start, unsigned int end,
    int (*comp)(const void *, const void *)
) {
    if (start >= end) {
        return ;
    }

    int med = (end+start)/2;
    int k = start;
    int j = end;
    char *tmp = NULL;
    char * b = base;
    for (int i=0;i<size;i++) {
        SWAP((b+i)[med],(b+i)[start]);
    }

    for(j=start+1;j<=end;j+=size) {
        if (comp(b[j*size],b[start*size]) < 0) {
            k++;
            if (k==j)continue;
            for(int i=0;i<size;i++) {
                SWAP((b+i)[k*size],(b+i)[j*size]);
            }
        }
    }

    SWAP(d[i],d[start]);

    qsortstr(d, start, i-1);
    qsortstr(d, i+1,end);
}
