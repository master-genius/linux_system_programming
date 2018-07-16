#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

char *_path[] = {
    "\0",
    "/bin",
    "/sbin",
    "/usr/bin",
    "/usr/sbin",
    "/usr/local/bin",
    "/usr/local/sbin"
};

#define ARGS_END    1024

#define MAX_NAME_LEN    2048

int _args_ind;

char * _args_p[ARGS_END] = {NULL,};

char _cmd_path[MAX_NAME_LEN];

int find_command(char * dir_list[], int n, char* name);

int main(int argc, char * argv[])
{
    _args_ind = 0;
    char * path = getenv("HOME");
    char home_bin[256] = {'\0'};
    strcpy(home_bin, path);
    strcat(home_bin, "/bin");
    
    struct stat st;

    if (access(home_bin,F_OK|R_OK|X_OK)==0
        && lstat(home_bin, &st)==0
        && S_ISDIR(st.st_mode)
    ) {
        _path[0] = home_bin;
    } else {
        home_bin[0] = '\0';
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

