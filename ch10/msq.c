#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

#define MSG_INUM    1
#define MSG_STR     2
#define MSG_DNUM    3
#define MSG_MAX     4

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

    time_t tm = time(NULL);
    srand(tm);
    for(int i=0; i<len; i++) {
        rstr[i] = total_str[rand() % tlen];
        srand(i+1 + tm);
    }

    return rstr;
}

int push_msg(int msg_type) {

    int mid = msgget(MSG_KEY, IPC_CREAT|0664);

    if (mid < 0) {
        perror("msgget");
        return -1;
    }

    struct msg_queue msq;

    time_t tm = time(NULL);

    if (msg_type == MSG_INUM) {
        srand(msg_type);
        for (int i=0; i<100; i++) {
            msq.msg_type = MSG_INUM;
            msq.msg.n = rand() % 8192;
            srand(i + tm);
            if (msgsnd(mid, &msq, sizeof(msq.msg), 0) < 0) {
                perror("msgsnd");
            }
        }
    } else if (msg_type == MSG_DNUM) {
        srand(tm - 1);
        for(int i=0; i<100; i++) {
            msq.msg_type = MSG_DNUM;
            msq.msg.d = ((double)i) * 3.14 + (double)(rand() % 1024);
            if (msgsnd(mid, &msq, sizeof(msq.msg), 0) < 0) {
                perror("msgsnd");
            }
            srand(tm + i);
        }
    } else if (msg_type == MSG_STR) {

        for(int i=0; i<100; i++) {
            msq.msg_type = MSG_STR;
            strcpy(msq.msg.name, rand_str(((i*i)%41) + 3));
            if (msgsnd(mid, &msq, sizeof(msq.msg), 0) < 0) {
                perror("msgsnd");
            }
        }
    } else {
    
    }

    return 0;
}

int msq_recv(int msg_type) {

    int mid = msgget(MSG_KEY, 0664|IPC_CREAT);

    if (mid < 0) {
        return -1;
    }

    struct msg_queue msq;

    int msg_count = 1;

    while(1) {
        if (msgrcv(mid, &msq, sizeof(msq.msg), msg_type, 0) < 0) {
            break;
        }

        printf("%d--%-3d ", getpid(), msg_count++);
        if (msq.msg_type == MSG_INUM)
            printf("get msg : %d\n", msq.msg.n);
        else if (msq.msg_type == MSG_DNUM)
            printf("get msg : %.2lf\n", msq.msg.d);
        else if (msq.msg_type == MSG_STR)
            printf("get msg : %s\n", msq.msg.name);
    }

    return 0;
}


/*
    args :
        send [int|double|str|all]
        recv [int|double|str|all]
        msqrm
*/

#define HELP_INFO   "\
usage msq [INSTRUCTION] [MSG TYPE]\n\
    instruction : send recv msqrm\n\
    msg type [int|double|str|all]\n\
    example:\n\
        msq send all\n\
        msq send str\n\
        \n\
        msq recv str\n\
        msq msqrm\n\
"

void help() {
    printf("%s\n", HELP_INFO);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        help();
        return 0;
    }

    if (argc < 3) {
        if (strcmp(argv[1], "msqrm") == 0) {
            int mid = msgget(MSG_KEY, 0664); 
            if (mid > 0 && msgctl(mid, IPC_RMID, NULL) < 0) {
                perror("msgctl");
                return -1;
            }
            return 0;
        }

        return -1;
    }

    int msg_type = 0;

    if (strcmp(argv[2], "all") == 0) {
        msg_type = 0;
    } else if (strcmp(argv[2], "int") == 0) {
        msg_type = MSG_INUM;
    } else if(strcmp(argv[2], "double") == 0) {
        msg_type = MSG_DNUM;
    } else if (strcmp(argv[2], "str") == 0) {
        msg_type = MSG_STR;
    } else {
        dprintf(2, "Error: unknow msg type\n");
        return -1;
    }

    if (strcmp(argv[1], "send")==0) {
        if (msg_type == 0) {
            push_msg(MSG_INUM);
            push_msg(MSG_DNUM);
            push_msg(MSG_STR);
        }
        else
            push_msg(msg_type);
    } else if (strcmp(argv[1], "recv") == 0) {
        msq_recv(msg_type);
    } else {
        dprintf(2 , "Error : unknow command\n");
        return -1;
    }

	return 0;
}

