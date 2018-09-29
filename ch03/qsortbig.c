#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

#define HELP_INFO   "\
    排序性能测试程序：\n\
        对比插入排序和快速排序\n\
        程序默认随机生成10000个整数进行快速排序\n\
        --time   :  输出排序运行时间\n\
        --sort   :  设定排序方式，支持insert和quick\n\
        --no-out :  不输出排序结果\n\
        [NUMBER] :  设定一个要随机生成的整数个数\n\
    示例：\n\
        随机生成5000个数进行插入排序：\n\
            sortbig --sort=insert 5000\n\
        随机生成20000个数进行快速排序并输出运行时间：\n\
            sortbig 20000 --time [默认就是进行快速排序]\n\
        不输出排序结果：\n\
            sortbig --no-out 15000 --time --sort=insert\n\
"

void help(void) {
    printf("%s\n", HELP_INFO);
}

void qsorti(int *d, int start, int end) {
    if (start >= end) {
        return ;
    }

    int med = (end+start)/2;
    int i = start;
    int j = end;
    int tmp = 0;

    tmp = d[med];
    d[med] = d[start];
    d[start] = tmp;

    for(j=start+1;j<=end;j++) {
        if (d[j]<d[start]) {
            i++;
            if (i==j)continue;
            tmp = d[i];
            d[i] = d[j];
            d[j] = tmp;
        }
    }

    tmp = d[i];
    d[i] = d[start];
    d[start] = tmp;

    qsorti(d, start, i-1);
    qsorti(d, i+1,end);
}

void insert_sort(int *d, int n) {
    int tmp = 0;
    int j,k;
    for(k=1;k<n;k++){
        tmp = d[k];
        for(j=k;j>0 && d[j-1]>tmp;j--)
            d[j] = d[j-1];
        d[j] = tmp;
    }
}

#define ARGS_OUT        0
#define ARGS_SORT       1
#define ARGS_TIME       2

#define ARGS_END        3

#define SORT_INSERT     'i'
#define SORT_QUICK      'q'

int main(int argc, char *argv[])
{
    int N = 10000;
    char ARGS[ARGS_END] = {0,};
    ARGS[ARGS_OUT] = 1;
    ARGS[ARGS_SORT] = SORT_QUICK;

    if (argc>1) {
        for(int i=1;i<argc;i++) {
            if (strcmp(argv[i], "--help") == 0) {
                help();
                return 0;
            } else if (strncmp(argv[i],"--sort=",7)==0) {
                if (strcmp(argv[i]+7, "insert")==0) {
                    ARGS[ARGS_SORT] = SORT_INSERT;
                } else if(strcmp(argv[i]+7,"quick")==0) {
                    ARGS[ARGS_SORT] = SORT_QUICK;
                }
            }
            else if (strcmp(argv[i],"--no-out")==0){
                ARGS[ARGS_OUT] = 0;
            }
            else if (strcmp(argv[i],"--time")==0) {
                ARGS[ARGS_TIME] = 1;
            }
            else {
                N = atoi(argv[i]);
                N = (N<10)?10000:N;
            }
        }
    }
    
    int *nms = (int*)malloc(sizeof(int)*N);
    if (nms==NULL) {
        perror("malloc");
        return -1;
    }

    printf("creating random number...\n");
    srand(time(NULL));
    for(int i=0;i<N;i++) {
        nms[i] = (N-i) + (rand() % 1024);
        srand(i);
    }

    /*
        获取开始排序的时间
    */
    struct timeval  start_time,end_time;
    gettimeofday(&start_time,0);

    if (ARGS[ARGS_SORT] == SORT_QUICK) {
        qsorti(nms, 0, N-1);
    } else {
        insert_sort(nms, N);
    }

    /*
        获取结束时间计算运行时间
    */
    gettimeofday(&end_time,0);
    double run_sec = (double)(end_time.tv_sec - start_time.tv_sec);
    double run_usec = ((double)(end_time.tv_usec - start_time.tv_usec)/1000000);

    if (ARGS[ARGS_OUT]) {
        for (int i=0;i<N;i++) {
            printf("%-6d ", nms[i]);
            if ( ( (i+1)%6 ) ==0 )
                printf("\n");
        }
        printf("\n");
    }

    if (ARGS[ARGS_TIME]) {
        printf("--run time : %g s\n", run_sec+run_usec);
    }

    free(nms);
    nms = NULL;

    return 0;
}

