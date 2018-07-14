
#define SWAP(a,b)   tmp=a;a=b;b=tmp;

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
            if (k==j)continue;
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

