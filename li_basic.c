#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#define TYPE_DIR        '/'
#define TYPE_LNK        '@'
#define TYPE_FIFO       '|'
#define TYPE_SOCK       '='
#define TYPE_EXEC       '*'
#define TYPE_CHR        '%'
#define TYPE_BLK        '#'


#define ARGS_PATH       0
#define ARGS_SIZE       1
#define ARGS_LINK       2
#define ARGS_TYPE       3    //char for file type
#define ARGS_MODE       4
#define ARGS_LSALL      5
#define ARGS_INO        6
#define ARGS_LONGINFO   7
#define ARGS_HELP       8

#define ARGS_END        32

#define MAX_NAME_LEN    2048

char _args[ARGS_END] = {0};


int list_fildir(char * pathname);

void help(void);

void out_info(char * pathname, char * name, struct stat * st);

int main(int argc, char *argv[])
{
    char _path_buffer[MAX_NAME_LEN] = {'\0'};
    int len_buf = 0;
    int args_dir_count = 0;

    int * fil_ind = malloc(sizeof(int)*argc);
    if (fil_ind ==NULL) {
        perror("malloc");
        return 1;
    }
    bzero((void*)fil_ind, sizeof(int)*argc);

    for(int i=1;i<argc;i++) {
        if (strcmp(argv[i],"--lnk")==0) {
            _args[ARGS_LINK] = 1;
        } else if (access(argv[i], F_OK)==0) {
            len_buf = strlen(argv[i]);
            if (len_buf >= MAX_NAME_LEN) {
                dprintf(2,"Error: the length of path too long -> %s\n",argv[i]);
                continue;
            }
            fil_ind[i] = 1;
            args_dir_count += 1;
        } else if (argv[i][0]=='-') {
            len_buf = strlen(argv[i]);
            for(int a=1; a<len_buf; a++){
                if (argv[i][a]=='a')
                    _args[ARGS_LSALL] = 1;
                else if (argv[i][a] == 'i')
                    _args[ARGS_INO] = 1;
                else if (argv[i][a] == 'm')
                    _args[ARGS_MODE] = 1;
                else if (argv[i][a] == 'p')
                    _args[ARGS_PATH] = 1;
                else if (argv[i][a] == 's')
                    _args[ARGS_SIZE] = 1;
                else if (argv[i][a] == 'f')
                    _args[ARGS_TYPE] = 1;
                else if (argv[i][a] == 'l')
                    _args[ARGS_LONGINFO] = 1;
                else if (argv[i][a] == 'h') {
                    _args[ARGS_HELP] = 1;
                }
      
            }
        }
        else
            dprintf(2, "Error:unknow arguments -> %s\n", argv[i]);
    }
    
    if (_args[ARGS_HELP]) {
        help();
    } else if (args_dir_count == 0) {
        list_fildir(".");
    } else {
        for(int i=1; i<argc; i++) {
            if (fil_ind[i]==1) {
                list_fildir(argv[i]);
            }
        }
    }

    free(fil_ind);
    fil_ind = NULL;

    return 0;
}

int list_fildir(char* pathname) {
    struct stat st, stbuf;
    DIR* d = NULL;
    struct dirent * rd = NULL;

    char fbuf[MAX_NAME_LEN] = {'\0'};

    if (lstat(pathname, &st)<0) {
        perror("lstat");
        return -1;
    }

    int path_len = strlen(pathname);
    
    strcpy(fbuf, pathname);
    if (pathname[path_len] != '/'  
        && (path_len > 1 || pathname[0]!='/')
    ) {
        strcat(fbuf, "/");
        path_len += 1;
    }

    if (S_ISDIR(st.st_mode)) {
        d = opendir(pathname);
        if (d==NULL) {
            perror("opendir");
            return -1;
        }
        while((rd = readdir(d))!=NULL) {
            if (strlen(pathname) + strlen(rd->d_name) + 1 >= MAX_NAME_LEN) {
                dprintf(2, "Error: name too long\n");
                continue;
            }
            if (_args[ARGS_LSALL]==0 && rd->d_name[0]=='.')
                continue;

            fbuf[path_len] = '\0';
            strcat(fbuf, rd->d_name);

            if (lstat(fbuf, &stbuf)<0) {
                perror("lstat");
                continue;
            }
            out_info(fbuf, rd->d_name, &stbuf);
        }
        closedir(d);
    } else {
        out_info(pathname, pathname, &st);
    }
    
    return 0;
}

void out_info(char * pathname, char * name, struct stat * st) {
    if (_args[ARGS_INO])
        printf("%-8lu ", st->st_ino);
    
    if(_args[ARGS_MODE] || _args[ARGS_LONGINFO])
        printf("%o ", st->st_mode & 0777);

    if (_args[ARGS_LONGINFO]) {
        printf("%-2lu %d %d ", st->st_nlink, st->st_uid, st->st_gid);
    }

    if (_args[ARGS_PATH])
        printf("%s", pathname);
    else
        printf("%s", name);

    char flag = '\0';
    if (_args[ARGS_TYPE]) {
        if (S_ISDIR(st->st_mode))
            flag = TYPE_DIR;
        else if (S_ISLNK(st->st_mode))
            flag = TYPE_LNK;
        else if (S_ISFIFO(st->st_mode))
            flag = TYPE_FIFO;
        else if (S_ISSOCK(st->st_mode))
            flag = TYPE_SOCK;
        else if (S_ISCHR(st->st_mode))
            flag = TYPE_CHR;
        else if (S_ISBLK(st->st_mode))
            flag = TYPE_BLK;
        else if (S_ISREG(st->st_mode) && access(pathname,X_OK)==0)
            flag = TYPE_EXEC;
        if (flag > 0)
            printf("%c",flag);
    }

    if (_args[ARGS_SIZE] || _args[ARGS_LONGINFO]) {
        if (st->st_size <= 1024)
            printf(" %luB",st->st_size);
        else if (st->st_size > 1024 && st->st_size < 1048576)
            printf(" %.2lfK", (double)st->st_size/1024);
        else
            printf(" %.2lfM",(double)st->st_size/1048576);
    }

    if (S_ISLNK(st->st_mode) 
        && (_args[ARGS_LONGINFO] || _args[ARGS_LINK])
    ) {
        char _path_buffer[MAX_NAME_LEN];
        int link_len = readlink(pathname, _path_buffer, MAX_NAME_LEN-1);
        if (link_len > 0) {
            _path_buffer[link_len] = '\0';
            printf("-> %s ", _path_buffer);
        }
    }

    printf("\n");
}

void help(void)
{
    char *help_info[] = {
        "显示文件/目录信息,类似于ls命令。\n",
        "参数/选项：\n",
        "-l ：显示详细信息，包括权限、用户、用户组、大小等信息；\n-m ：权限\n",
        "-a ：显示隐藏文件；\n-i ：i-node编号；\n-s ：大小；\n-f ：文件类型字符\n",
        "-p ：显示路径；\n--lnk ：链接目标文件\n",
        "示例：\n",
        "li -al /usr \nli -rfs /usr/share /usr/local\n",
        "\n",
        "\0"
    };
    int i=0;
    while (strcmp(help_info[i],"\0")!=0) {
        printf("%s",help_info[i++]);
    }
}

