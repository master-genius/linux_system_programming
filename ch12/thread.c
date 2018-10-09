#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>


char _data[128] = "abcdefg";
int _stcode = -1;


void * thread_a (void* val) {
    for(int i=0; i<100; i++) {
        printf("%s\n", _data);
        fflush(stdout);
        usleep(100000);
    }

    _stcode = 0;
    //int status[2] = {0,0};
    //pthread_exit(&status);

    return NULL;
}


int main(int argc, char *argv[]) {

    pthread_t pthd;

    pthread_create(&pthd, NULL, thread_a, NULL);

    //int exit_info[2] = {-1,-1};
    //int *status[2];
    //status[0] = exit_info;
    //status[1] = exit_info+1;
    //pthread_join(pthd, (void**)status);

    int len = strlen(_data);
    char c;
    for(int i=0; i<150; i++) {
        c = _data[i%len];
        c++;
        if (c > 120)
            c = '.';
        _data[i%len] = c;
        printf("--%s\n", _data);
        fflush(stdout);
        usleep(150000);
    }

    while (_stcode >= 0) {
        usleep(100000);
        break;
    }


	return 0;
}
