#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>

char *_path[] = {
    NULL,
    "/bin",
    "/sbin",
    "/usr/bin",
    "/usr/sbin",
    "/usr/local/bin",
    "/usr/local/sbin"
};

char _home_bin[256] = {'\0'};

#define ARGS_END    1024

#define MAX_NAME_LEN    2048

#define BUILD_NOTFD     -1
#define BUILD_OK        0
#define BUILD_ERR       1

int _args_ind;

char * _args_p[ARGS_END] = {NULL,};

char _cmd_path[MAX_NAME_LEN];

int find_command(char * dir_list[], int n, char* name);

int build_in(char * cmd, char * cmd_argv[]);

void handle_sig(int sig) {
    printf("\n");
}

int main(int argc, char * argv[])
{
    signal(SIGINT, handle_sig);

    _args_ind = 0;
    char * path = getenv("HOME");
    if (path) {
        strcpy(_home_bin, path);
        strcat(_home_bin, "/bin");
    }
    
    struct stat st;

    if (access(_home_bin,F_OK|R_OK|X_OK)==0
        && lstat(_home_bin, &st)==0
        && S_ISDIR(st.st_mode)
    ) {
        _path[0] = _home_bin;
    }

    int pid = 0;
    char cmd_buf[8192] = {'\0'};
    int count = 0;
    char **cmd_argv = NULL;
    int i;
    while (1) {
        write(1, "|==>", 4);
        count = read(0,cmd_buf,8191);
        if (count<0) {
            perror("read");
            continue;
        } else {
            cmd_buf[count-1] = '\0';
            _args_ind = 0;
            _args_p[_args_ind] = strtok(cmd_buf, " ");
            if (_args_p[_args_ind]!=NULL) {
                while (_args_ind < ARGS_END) {
                    _args_ind++;
                    _args_p[_args_ind] = strtok(NULL, " ");
                    if (_args_p[_args_ind]==NULL)break;
                }
            } else {
                _args_ind = 0;
            }
            
            if (_args_p[0]==NULL || strlen(_args_p[0])==0)
                continue;
        }
        
        cmd_argv = (char**)malloc(sizeof(char*)*(_args_ind+1));
        if (cmd_argv == NULL) {
            perror("malloc");
            continue;
        }
        for(i=0;i<_args_ind;i++)
            cmd_argv[i] = _args_p[i];

        cmd_argv[_args_ind] = NULL;

        if (build_in(cmd_argv[0], cmd_argv+1) != BUILD_NOTFD) {
            continue;
        }

        if (find_command(_path, sizeof(_path)/sizeof(char*), cmd_argv[0])) {
            printf("Error: command not found -> %s\n", cmd_argv[0]);
            continue;
        }

        pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid > 0) {
            int status = 0;
            wait(&status);
            free(cmd_argv);
            cmd_argv = NULL;
        }

        if (pid == 0) {
            if (execv(_cmd_path, cmd_argv)<0) {
                perror("execv");
                return -1;
            }
        }
    }

    return 0;
}

int find_command(char * dir_list[], int n, char * name) {

    DIR * d = NULL;
    struct dirent * rd;

    for (int i=0; i<n; i++) {
        
        if (dir_list[i]==NULL)continue;

        d = opendir(dir_list[i]);
        if (d==NULL) {
            perror("opendir");
            continue;
        }
        while((rd = readdir(d))!=NULL) {
            if (strcmp(rd->d_name, "..")==0 || strcmp(rd->d_name, ".")==0)
                continue;
            if (strcmp(rd->d_name, name)==0) {
                strcpy(_cmd_path, dir_list[i]);
                strcat(_cmd_path, "/");
                strcat(_cmd_path, rd->d_name);
                closedir(d);
                return 0;
            }
        }
        closedir(d);
    }
    return -1;
}

int build_in(char * cmd, char * cmd_argv[]) {
    
    if (strcmp(cmd, "cd")==0) {
        if (cmd_argv[0]==NULL)
            chdir(getenv("HOME"));
        else {
            if (chdir(cmd_argv[0])<0) {
                perror("chdir");
                return BUILD_ERR;
            }
        }
    } else if (strcmp(cmd, "pwd")==0) {
        char cwd[MAX_NAME_LEN] = {'\0'};
        if (getcwd(cwd, MAX_NAME_LEN-1)==NULL) {
            perror("getcwd");
            return BUILD_ERR;
        }
        printf("%s\n",cwd);
    } else if (strcmp(cmd, "help")==0) {
        printf("There is no help\n");
    } else if (strcmp(cmd, "exit")==0 || strcmp(cmd, "quit")==0) {
        _exit(0);
    } else {
        return BUILD_NOTFD;
    }

    return BUILD_OK;
}

