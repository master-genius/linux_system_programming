#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <sys/wait.h>

#define MSG_INUM    0
#define MSG_STR     1
#define MSG_DNUM    2
#define MSG_MAX     3

#define MAX_NAME_LEN    2048

#define MSG_KEY     1248

struct msg_content {
    int n;
    double d;
    char name[MAX_NAME_LEN];
};


struct msg_queue {
    long msg_type;
    struct msg_content msg;
};

int _msg_id;
int _child_count;

void handle_sig(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        msgctl(_msg_id, IPC_RMID, NULL);
        kill(0, SIGTERM);
        usleep(50000);
        exit(0);
    } else if (sig == SIGCHLD) {
        int st;
        pid_t pid;

        while((pid=waitpid(0, &st, WNOHANG)) > 0) {
            if (WIFEXITED(st) || WIFSIGNALED(st)) {
                _child_count -= 1;
            }
        }
    }
}

#define MAX_RANDSTR_LEN    42

char * rand_str(int len) {
    
    char *total_str = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMPOPQRSTUVWXYZ0123456789";
    int tlen = strlen(total_str);

    static char rstr[MAX_RANDSTR_LEN+1] = {'\0',};

    for(int i=0; i<MAX_RANDSTR_LEN; i++)
        rstr[i] = '\0';

    if (len > MAX_RANDSTR_LEN)
        len = MAX_RANDSTR_LEN;
    else if (len <= 0)
        len = 5;

    srand(len);
    for(int i=0; i<len; i++) {
        rstr[i] = total_str[rand() % tlen];
        srand((i+1)*len);
    }

    return rstr;
}

int parent_push_msg() {
    int mid = msgget(MSG_KEY, IPC_CREAT|0664);

    struct msg_queue msq;

    msq.msg_type = MSG_INUM;
    for (int i=0; i<100; i++) {
        msq.msg.n = i;
        printf("----%d\n", msq.msg.n);
        msgsnd(mid, &msq, sizeof(msq.msg), 0);
    }

    msq.msg_type = MSG_DNUM;
    for(int i=0; i<100; i++) {
        msq.msg.d = ((double)i) * 3.14;
        msgsnd(mid, &msq, sizeof(msq.msg), 0);
    }

    msq.msg_type = MSG_STR;
    for(int i=0; i<100; i++) {
        strcpy(msq.msg.name, rand_str(((i*i)%41) + 3));
        msgsnd(mid, &msq, sizeof(msq.msg), 0);
    }

    return 0;
}

int child_msg_recv(int msg_type) {
    int mid = msgget(MSG_KEY, 0664);
    

    struct msg_queue msq;
    
    msq.msg_type = msg_type;


    int msg_count = 1;

    printf("child start to recv msg\n");
    while(1) {
        if (msgrcv(mid, &msq, sizeof(msq.msg), 0, 0) < 0) {
            break;
        }

        printf("%d--%-3d ", getpid(), msg_count++);
        if (msg_type == MSG_INUM)
            printf("get msg : %d\n", msq.msg.n);
        else if (msg_type == MSG_DNUM)
            printf("get msg : %.2lf\n", msq.msg.d);
        else if (msg_type == MSG_STR)
            printf("get msg : %s\n", msq.msg.name);
    }

    return 0;
}

#define PCS_CHILD   1
#define PCS_PARENT  2


int main(int argc, char *argv[]) {

    int mid = msgget(MSG_KEY, IPC_CREAT|0664);
    _msg_id = mid;
    _child_count = 0;

    int process_flag = PCS_PARENT;
    int msg_type = 0;

    pid_t pid;

    for (int i=0; i<MSG_MAX; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            return -1;
        }
        if (pid > 0) {
            _child_count += 1;
            continue;
        }

        if (pid == 0) {
            msg_type = i + 1;
            process_flag = PCS_CHILD;
            break;
        }

    }

    if (process_flag == PCS_CHILD) {
        usleep(100000);
        child_msg_recv(msg_type);
    } else {
        signal(SIGINT, handle_sig);
        signal(SIGTERM, handle_sig);
        signal(SIGCHLD, handle_sig);

        parent_push_msg();
        while(1) {
            if (_child_count <= 0)
                break;
            sleep(1);
        }

        int mid = msgget(MSG_KEY, 0664); 
        msgctl(mid, IPC_RMID, NULL);
    }

	return 0;
}

