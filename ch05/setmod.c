#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>


#define HELP_INFO   "\
setmod : 设置文件的权限，类似于chmod\n\
    setmod可以支持--auto选项，用于自动识别文件类型,目录会自动加上可执行权限，\
文件会自动去掉可执行权限。\n\
    --auto        : 启用自动模式\n\
    --auto=[u|ug] : 自动模式设置只针对当前用户或者是用户和用户组\n\
        这在设置目录时会起作用，如果你设置--auto=ug 644这意味着\n\
        对于用户和用户组来说目录会自动加上可执行权限，而其他用户不会\n\
    -r            : 递归设置目录\n\
    [MODE]        : 三位数表示8进制的权限码\n\
    \
example:\n\
    setmod -r --auto 755 m/\n\
    setmod -r 644 m/\n\
"

void help() {
    printf("%s", HELP_INFO);
}

#define MAX_NAME_LEN    2048

#define AUTO_ALL        1
#define AUTO_USR        2
#define AUTO_UG         3


#define ARGS_AUTO       0
#define ARGS_RECUR      1
#define ARGS_CHLNK      2
#define ARGS_IGNSELF    3

#define ARGS_END        8

char __args[ARGS_END];
int __auto_dir_mode;

struct path_list {
    struct path_list *next;
    struct path_list *prev;
    struct path_list *last;
    int height;
    int len;
    int count;
    char pname[MAX_NAME_LEN+1];
};


struct path_list *
init_path_list(struct path_list* pl) {
   pl->next = NULL;
   pl->prev = NULL;
   pl->last = pl;
   pl->height = 1;
   pl->count = 0;
   pl->len=0;
   return pl;
}

void destroy_path_list(struct path_list * pl) {
    struct path_list * ptmp;
    ptmp = pl;

    while(pl!=NULL) {
        ptmp = pl->next;
        free(pl);
        pl = ptmp;
    }
}

struct path_list *
add_path_list(struct path_list * pl, char * path, int len, int height);

void
del_path_list(struct path_list * pl, struct path_list * pnode);

struct path_list *
get_path_list_last(struct path_list * pl);


struct path_list *
add_path_list(struct path_list * pl, char * path, int len, int height) {
    struct path_list * plast =  pl->last;
    struct path_list * ptmp;

    ptmp = (struct path_list*)malloc(sizeof(struct path_list));
    if (ptmp==NULL) {
        perror("malloc");
        return NULL;
    }

    plast->next = ptmp;
    ptmp->next = NULL;
    ptmp->prev = plast;
    pl->last = ptmp;
    ptmp->count = plast->count + 1;


    strcpy(ptmp->pname, path);
    if (len > 0)
        ptmp->len = len;
    else
        ptmp->len = strlen(path);

    if (height > 0) {
        ptmp->height = height;
    }

    return ptmp;
}


void
del_path_list(struct path_list * pl, struct path_list * pnode) {
    if (pnode == NULL)return ;

    //如果是首节点则不释放
    if (pnode->prev == NULL)return ;

    pnode->prev->next = pnode->next;
    
    if (pnode->next)
        pnode->next->prev = pnode->prev;

    if (pl->last == pnode) {
        pl->last = pnode->prev;
    }
    free(pnode);
}

int set_mode(char *p, int mode, struct stat *st) {
    
    int mode_buf = mode;

    switch (st->st_mode & S_IFMT) {
        case S_IFDIR:
            if(__args[ARGS_AUTO]) {
                mode_buf |= __auto_dir_mode;       
            }
            if (mode_buf != (st->st_mode&0777))
                return chmod(p, mode_buf);
            break;
        case S_IFREG:
        case S_IFIFO:
        case S_IFBLK:
        case S_IFCHR:
        case S_IFSOCK:
            if (__args[ARGS_AUTO]) {
                mode_buf &= 0666;
            }
            if (mode_buf != (st->st_mode&0777))
                return chmod(p, mode_buf);
            break;
        case S_IFLNK:
            if (__args[ARGS_CHLNK]) {
            
            }
            break;

        default:;
    
    }

    return 0;
}


#define LINUX_NAME_LEN      256

int recur_dir(struct path_list*ph, int mode, int deep) {
    if (ph == NULL)
        return 0;

    struct path_list *pl = ph->next;
    struct path_list *pold = NULL;
    struct stat stmp;

    DIR * d = NULL;
    struct dirent *rd = NULL;

    int i,k;
    int cur_height = 1;

    char pathbuf[MAX_NAME_LEN+1] = {'\0'};
    unsigned long int len_buf = 0;

    while (pl) {
        cur_height = pl->height;
        if (deep > 0 && cur_height > deep)goto end_recur;

        d = opendir(pl->pname);
        if (!d) {
            perror("opendir");
            goto next_dir;
        }
        
        while((rd=readdir(d))!=NULL) {
            if (strcmp(rd->d_name, ".") == 0
                || strcmp(rd->d_name, "..") == 0
            ) {
                continue;
            }

            strcpy(pathbuf, pl->pname);
            if (!(pl->len==1 && pl->pname[0]=='/'))
                strcat(pathbuf, "/");

            if (pl->len + LINUX_NAME_LEN >= MAX_NAME_LEN)
                continue;
            
            strcat(pathbuf, rd->d_name);

            if (lstat(pathbuf, &stmp)<0) {
                perror("lstat");
                continue;
            }

            if (S_ISDIR(stmp.st_mode)) {
                add_path_list(ph, pathbuf, strlen(pathbuf), cur_height+1);
            }
            
            if (set_mode(pathbuf, mode, &stmp) < 0) {
                dprintf(2, "%s", pathbuf);
                perror("chmod");
                goto end_recur;
            }

        }//end readdir
        closedir(d);

      next_dir:;
        pold = pl;
        pl = pl->next;
        del_path_list(ph, pold);
    }

  end_recur:;

    return 0;
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        help();
        return 0;
    }

    for(int i=0; i<ARGS_END; i++)
        __args[i] = 0;

    struct path_list ph;
    init_path_list(&ph);
    int len = 0;
    int mode = 0;
    struct stat st;

    for (int i=1; i<argc; i++) {
        if (strncmp(argv[i], "--auto=", 7) == 0) {
            __args[ARGS_AUTO] = 1;

            if (strcmp(argv[i]+7, "u") == 0) {
                __auto_dir_mode = 0100;
            } else if (strcmp(argv[i]+7, "ug") == 0) {
                __auto_dir_mode = 0110;
            } else {
                dprintf(2, "Error: unknow auto type\n");
                return -1;
            }
        } else if (strcmp(argv[i], "--auto")==0) {
            __args[ARGS_AUTO] = 1;
            __auto_dir_mode = 0111;
        } else if (strcmp(argv[i], "-r")==0) {
            __args[ARGS_RECUR] = 1;
        } else if (argv[i][0] >= '0' && argv[i][0] <= '7') {
            if (mode > 0) {
                continue;
            }
            len = strlen(argv[i]);
            if (len != 3) {
                dprintf(2, "Error: mode wrong\n");
                return -1;
            }
            mode = strtol(argv[i], NULL, 8);
            if (mode <= 0) {
                dprintf(2, "Error: mode number wrong\n");
                return -1;
            }
            
        }else {
            if (argv[i][0] == '-') {
                dprintf(2, "Error: unknow args\n");
                return 1;
            }
            if (mode <= 0) {
                dprintf(2, "Error: mode not set\n");
                return -1;
            }

        }

    }

    int errcode = 0;

    for (int i=1; i<argc; i++) {
    
        if (strncmp(argv[i], "--auto=", 7) == 0) {

        } else if (strcmp(argv[i], "--auto")==0) {

        } else if (strcmp(argv[i], "-r")==0) {

        } else if (argv[i][0] >= '0' && argv[i][0] <= '7') {

        }else {
            if (access(argv[i], F_OK) < 0) {
                continue;
            }

            if (lstat(argv[i], &st) < 0) {
                perror("lstat");
                continue;
            }

            if (set_mode(argv[i], mode, &st) < 0) {
                errcode = -1;
                goto end_set_mode;
            }

            if (!S_ISDIR(st.st_mode)) {
                continue;
            }

            len = strlen(argv[i]);
            if (len > 1 && argv[i][len-1] == '/') {
                argv[i][len-1] = '\0';
                len -= 1;
            }
            add_path_list(&ph, argv[i], len, 1);
        }

    }

    recur_dir(&ph, mode, 0);

end_set_mode:;
    destroy_path_list(ph.next);

    return errcode;
}

