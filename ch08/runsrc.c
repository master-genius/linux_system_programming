#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define CHILD_PRE       0
#define CHILD_RUN       1
#define CHILD_OUT       2
#define CHILD_EXIT      3

int _child_pid = 0;
int _run_status = CHILD_PRE;
unsigned int _time_out = 3;

void kill_child(int sig) {
    if (_run_status == CHILD_PRE || _run_status == CHILD_RUN) {
        signal(SIGALRM, kill_child);
        alarm(_time_out);
    } else if (_child_pid > 0) {
        kill(_child_pid, SIGTERM);
        _run_status = CHILD_EXIT;
    }

}

void term_handle(int sig) {
    printf("get signal : sigterm -> %d\n",sig);
}

int main(int argc, char *argv[])
{
    char * program = NULL;

    for (int i=1; i<argc; i++) {
        if (strncmp("--time-out=",argv[i],11)==0) {
            _time_out = atoi(argv[i]+11);
            if (_time_out <= 0 || _time_out > 5) {
                _time_out = 3;
            }
        } else {
            program = argv[i];
        }
    }
    
    
    int pid = fork();
    if (pid<0){
        perror("fork");
        return -1;
    }
    
    if (pid > 0) {
        printf("start ...\n");
        _child_pid = pid;

        signal(SIGALRM, kill_child);
        alarm(_time_out);
    } else {
        signal(SIGTERM, term_handle);
        printf("child %d sleep...\n",getpid());
        sleep(8);
    }

    return 0;
}

