#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define ARGS_MKPARENT   0

#define ARGS_END        8

#define MK_ERR_NONE     0
#define MK_ERR_MALM     1
#define MK_ERR_FAIL     2
#define MK_ERR_ARGS     3

char _args[ARGS_END] = {'\0'};

int *_dir_ind = NULL;

void help(void)
{
    char *help_info[] = {
        "创建目录，支持参数--help，--mode\n",
        "--help：输出帮助信息\n",
        "--mode=[MODE]：设定创建目录的权限，MODE应该是一个三位的数字，",
        "否则程序会报错，数字是0-7的范围。比如，754表示rwxr-xr--。\n",
        "-p 如果父级目录不存在则创建。\n",
        "示例：\n",
        "    mkdir --mode=755 a/ b/ c/\n",
        "    mkdir study/\n",
        "    mkdir --help",
        "\n",
        "\0"
    };
    int i=0;
    while (strcmp(help_info[i],"\0")!=0) {
        printf("%s",help_info[i++]);
    }
}

int try_make_parent(char *path, int mode);

int recur_make_parent(char *path, int mode);

void clean_exit(int err) {
    if (_dir_ind) {
        free(_dir_ind);
    }

    exit(err);
}

int main(int argc, char *argv[])
{
    if (argc<2) {
        dprintf(2,"Error:less DIR_NAME\n");
        return -1;
    }

    int mode_flag = 0;
    int mode = 0755;
    int i;
    int mode_buf = 0;
    char tmp;
    int dir_count = 0;

    _dir_ind = (int*)malloc(sizeof(int)*(argc-1));
    if(_dir_ind == NULL) {
        perror("malloc");
        return MK_ERR_MALM;
    }

    for(int i=0; i<argc-1; i++)
        _dir_ind[i] = 0;

    for(i=1;i<argc;i++) {
        if (strcmp(argv[i],"--help")==0) {
            help();
            clean_exit(0);
        } else if (strncmp(argv[i],"--mode=",7)==0) {
            if (mode_flag > 0) {
                dprintf(2,"Error: too many --mode\n");
                clean_exit(MK_ERR_ARGS);
            }

            if (strlen(argv[i]+7)!=3) {
                dprintf(2, "Error: mode is wrong\n");
                clean_exit(MK_ERR_ARGS);
            }

            for (int k=0;k<3;k++) {
                tmp = argv[i][7+k];
                if (tmp < '0' || tmp > '7') {
                    dprintf(2, "Error: mode number must in [0,7]\n");
                    clean_exit(MK_ERR_ARGS);
                }
                mode_buf += (tmp-48)*(1<<(3*(2-k)));
            }

            mode_flag = i;
        } else if (strcmp(argv[i], "-p") == 0) {
            _args[ARGS_MKPARENT] = 1;
        } else {
            _dir_ind[i-1] = 1;
            dir_count ++;
        }
    }

    if (dir_count == 0) {
        dprintf(2,"Error: less DIR_NAME\n");
        clean_exit(MK_ERR_ARGS);
    }

    if (mode_flag>0 && mode_buf > 0)
        mode = mode_buf;

    for (i=1;i<argc;i++) {
        
        if (_dir_ind[i-1]==0)continue;

        if (mkdir(argv[i],mode) < 0) {
            if (_args[ARGS_MKPARENT]) {
                if (try_make_parent(argv[i], mode) < 0) {
                    clean_exit(MK_ERR_FAIL);
                }
            } else {
                perror("mkdir");
                return -1;
            }
        }
    }

    free(_dir_ind);
    _dir_ind = NULL;

    return 0;
}

int try_make_parent(char *path, int mode) {
    int plen = strlen(path);
    if (plen > 0 && path[plen-1]=='/')
        path[plen-1] = '\0';

    return recur_make_parent(path, mode);
}

int recur_make_parent(char *path, int mode) {
    
    int plen = strlen(path);
    if(plen<=0 || access(path, F_OK)==0)
        return 0;

    int i = plen-1;
    while(i>0 && path[i]!='/')i--;
    if (i == 0)
        goto start_mkdir;

    path[i] = '\0';

    if (access(path, F_OK) < 0)
        if(recur_make_parent(path, mode) < 0)
            return -1;

    path[i] = '/';

  start_mkdir:;
    if (mkdir(path, mode) < 0) {
        dprintf(2, "%s:\n", path);
        perror("mkdir");
        return -1;
    }

    return 0;
}

