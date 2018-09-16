#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#define PCS_CHILD   1
#define PCS_PARENT  2

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

key_t _save_semid = 0;

void handle_sig(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        printf("remove semid\n");
        semctl(_save_semid, 0, IPC_RMID);
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    
    int process_flag = PCS_PARENT;

    key_t semk = 1024;
    int semid;
    semid = semget(semk, 1, IPC_CREAT|0666);
    if ( semid < 0 ) {
        perror("semget");
        return 1;
    }
    _save_semid = semid;

    union semun semarg;

    struct sembuf mf;
    mf.sem_num = 0;
    mf.sem_op = 1;
    mf.sem_flg = SEM_UNDO;

    semarg.val = 1;
    semarg.buf = NULL;
    semarg.array = NULL;
    semarg.__buf = NULL;

    if (semctl(semid, 0, SETVAL, semarg)<0) {
        semctl(semid, 0, IPC_RMID);
        perror("semctl");
        return 2;
    }

    semop(semid, &mf, 1);

    pid_t pid;

    for(int i=0; i<2; i++) {
        pid = fork();
        if(pid<0) {
            perror("fork");
            kill(-1, SIGTERM);
        }
        if (pid > 0) {
            continue;
        } else {
            process_flag = PCS_CHILD;
            break;
        }
    }

    if (process_flag == PCS_PARENT) {
        int st=0;
        signal(SIGINT, handle_sig);
        for(int i=0; i<3; i++) {
            wait(&st);
        }
    } else {
        /*
            检测信号量是否为0，不为0则减1并输出信息，
            否则等待信号量大于0。
        */
        int ret;

        while (1) {
            //printf("%d \n", semctl(semid, 0, GETVAL));
            //printf("%u waiting\n", getpid());

            mf.sem_op = -1;
            mf.sem_flg |= IPC_NOWAIT | SEM_UNDO;
            
            ret = semop(semid, &mf, 1);
            if (ret < 0) {
                //printf("errno : %d\n", errno);
                //perror("semop");
            } else if (ret == 0) {
                printf("%u : say hey!\n", getpid());
                usleep(500000);
                printf("%u : done.\n", getpid());
                
                mf.sem_op = +1;
                mf.sem_flg = SEM_UNDO;
                if (semop(semid, &mf, 1)<0) {
                    perror("semop");
                }
            }
            sleep(1);
        }
    }


	return 0;
}
