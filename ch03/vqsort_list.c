#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct str_list {
    struct str_list *next;
    char name[256];
};


#define SWAP(a,b)   tmp=a;a=b;b=tmp;

void qsort_core(void * base, int start, int end,
    unsigned int size, int (*comp)(const void *, const void *)
);

void vqsort(void* base, unsigned int nmemb, unsigned int size, 
    int(*comp)( const void *, const void *)
) {
    qsort_core(base, 0, nmemb/size - 1, size, comp);
}


int strlist_comp(const void *a, const void *b) {
    const struct str_list * sa = *(struct str_list **)a;
    const struct str_list * sb = *(struct str_list **)b;
    return strcmp(sa->name, sb->name);
}


int main(int argc, char *argv[])
{
    if (argc < 2) {
        dprintf(2, "Error: enter some string\n");
        return -1;
    }

    int N = argc - 1;

    struct str_list strlhead;
    struct str_list *sl = NULL, *st = NULL;

    st = &strlhead;
    st->next = NULL;

    for(int i=1; i<argc; i++) {
        sl = (struct str_list*)malloc(sizeof(struct str_list));
        if (sl==NULL) {
            break;
        }
        strcpy(sl->name, argv[i]);
        st->next = sl;
        sl->next = NULL;
        st = st->next;
    }

    struct str_list **stra = (struct str_list**)malloc(sizeof(struct str_list*)*N);
    if(stra==NULL){
        perror("malloc");
    }

    sl = strlhead.next;
    int i=0;
    while(sl) {
        stra[i++] = sl;
        sl = sl->next;
    }


    vqsort(stra, sizeof(struct str_list*)*N, sizeof(struct str_list*), strlist_comp);

    for(i=0;i<N;i++) {
        printf("%s ", stra[i]->name);
    }printf("\n");

    sl = strlhead.next;
    while(sl) {
        st = sl->next;
        free(sl);
        sl = st;
    }

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

