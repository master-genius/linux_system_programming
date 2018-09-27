#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SWAP(a,b)   tmp=a;a=b;b=tmp;

void qsorti(int *d, int start, int end, int deep) {
    if (start >= end) {
        return ;
    }

    int med = (end+start)/2;
    int i = start;
    int j = end;
    int tmp = 0;
    
    printf("deep: %d; %d -> %d\n", deep, med, d[med]);

    SWAP(d[med],d[start]);

    for(j=start+1;j<=end;j++) {
        if (d[j]<d[start]) {
            i++;
            if (i==j)continue;
            SWAP(d[i],d[j]);
        }
    }

    SWAP(d[i],d[start]);

    for (int k=start; k<=end; k++)
        printf("%d ", d[k]);
    printf("\n\n");

    qsorti(d, start, i-1, deep+1);
    qsorti(d, i+1,end, deep+1);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        dprintf(2, "Error: enter some numbers\n");
        return -1;
    }

    int N = argc - 1;
    
    int *nms = (int*)malloc(sizeof(int)*N);
    if (nms==NULL) {
        perror("malloc");
        return -1;
    }

    for(int i=0;i<N;i++) {
        nms[i] = atoi(argv[i+1]);
    }

    qsorti(nms, 0, N-1, 1);

    for (int i=0;i<N;i++) {
        printf("%d ", nms[i]);
    }
    printf("\n");

    free(nms);
    nms = NULL;

    return 0;
}

