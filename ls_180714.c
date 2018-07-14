#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <math.h>
#include <pwd.h>
#include <grp.h>
#include <regex.h>
#include <time.h>

#define TYPE_DIR        '/'
#define TYPE_LNK        '@'
#define TYPE_FIFO       '|'
#define TYPE_SOCK       '='
#define TYPE_EXEC       '*'
#define TYPE_CHR        '%'
#define TYPE_BLK        '#'

#define SORT_BYNAME     'n'
#define SORT_BYTM       't'
#define SORT_BYCHTM     'c'
#define SORT_BYSIZE     's'

#define SORT_TY_CELL    'd'
#define SORT_TY_ALL     'a'


#define ARGS_PATH       0
#define ARGS_SIZE       1
#define ARGS_LINK       2
#define ARGS_TYPE       3    //char for file type
#define ARGS_MODE       4
#define ARGS_USR        5
#define ARGS_GRP        6
#define ARGS_CREATTM    7    //create time
#define ARGS_CHANGTM    8    //last change time
#define ARGS_ACCESTM    9    //last access time
#define ARGS_REGEX      10
#define ARGS_RECUR      11    //recursion
#define ARGS_STATIS     12    //statistic
#define ARGS_DIRSELF    13
#define ARGS_REGDIR     14
#define ARGS_REGFIL     15
#define ARGS_SORT       16
#define ARGS_NODIR      17    //not list dir, just file
#define ARGS_SORT_TYPE  18    //sort type : sort in cell of dir or sort all file
#define ARGS_NOTOUT     19    //do not output file info , 
                                //with ARGS_STATIS, just show the statis
#define ARGS_ONELINE    20    //show info in one line
#define ARGS_LSALL      21
#define ARGS_LSHIDD     22
#define ARGS_INO        23
#define ARGS_LONGINFO   24
#define ARGS_SHOWSTATS  25
#define ARGS_COLOR      26

#define ARGS_END        32

#define MAX_NAME_LEN    2048

char _args[ARGS_END] = {0};

struct path_list {
    char path[MAX_NAME_LEN];
    int is_dir;
    int height;
    struct path_list * next;
};


struct file_buf {
    char name[256];
    char path[MAX_NAME_LEN];
    char uname[40];
    char group[40];
    struct stat st;
};


#define BUFF_LEN    512

struct file_buf_list {
    struct file_buf fbuf;
    int count;
    struct file_buf_list * next;
};

struct file_list_cache {
    int end_ind;
    struct file_buf fcache[BUFF_LEN+1];
};

struct file_list_cache _file_cache;

struct file_buf_list _fbuf_list_head;
struct file_buf_list * _fbuf_list_end;

struct statis {
    unsigned long long dir_count;
    unsigned long long file_count;
    unsigned long long fifo_count;
    unsigned long long link_count;
    unsigned long long sock_count;
    unsigned long long chr_count;
    unsigned long long blk_count;
    unsigned long long total_count;
    unsigned long long total_size;
    unsigned long long file_total_size;
};

struct format_info {
    int count;
    int name_max_len;
    int uname_max_len;
    int group_max_len;
};

regex_t _regcom[1];
regmatch_t _regmatch[1];

#define SWAP(a,b)   tmp=a;a=b;b=tmp;

void qsortfbuf(struct file_buf *d[], int start, int end) {
    if (start >= end) {
        return ;
    }

    int med = (end+start)/2;
    int i = start;
    int j = end;
    struct file_buf *tmp = NULL;
    
    SWAP(d[med],d[start]);

    for(j=start+1;j<=end;j++) {
        if (strcmp(d[j]->name,d[start]->name) < 0) {
            i++;
            if (i==j)continue;
            SWAP(d[i],d[j]);
        }
    }

    SWAP(d[i],d[start]);

    qsortfbuf(d, start, i-1);
    qsortfbuf(d, i+1,end);
}

int init_path_list(char * path, struct path_list* pl);

int recur_dir(struct path_list* pl, int max_height, struct statis* stats);

void out_info(struct file_buf * fb, struct format_info fi);

void out_statis(struct statis* stats);

void start_statis(struct stat sttmp, struct statis * stats);

int out_control(char* path, struct format_info fi);

void destroy_path_list(struct path_list* pl);

void reset_file_buf_list();

void destroy_file_list(struct file_buf_list * fl);

void format_size(unsigned int size, char * fstr);

void help(void)
{
    char *help_info[] = {
        "显示文件/目录信息,类似于ls命令。\n",
        "参数/选项：-l -a -r -s -m -f -i -p -t --deep --lnk --regex\n",
        "-l ：显示详细信息，包括权限、用户、用户组、大小等信息；-m ：权限\n",
        "-a ：显示隐藏文件；-i ：i-node编号；-s ：大小；-f ：文件类型字符\n",
        "-r ：递归显示；--deep ：设置递归深度，--deep=5；--lnk ：链接目标文件\n",
        "-t ：统计信息；-p ：显示路径\n",
        "--regex [REGEX] ：使用正则表达式搜索文件，注意这不会对目录生效\n",
        "\n",
        "\0"
    };
    int i=0;
    while (strcmp(help_info[i],"\0")!=0) {
        printf("%s",help_info[i++]);
    }
}

int main(int argc, char *argv[])
{
    _file_cache.end_ind = -1;
    _fbuf_list_head.count = 0;
    _fbuf_list_head.next = NULL;
    _fbuf_list_end = &_fbuf_list_head;

    struct path_list pl;
    pl.next = NULL;

    struct statis stats;

    bzero(&stats, sizeof(stats));

    char _path_buffer[MAX_NAME_LEN] = {'\0'};
    
    int recur_deep = 1;
    int len_buf = 0;
    for(int i=1;i<argc;i++) {
        if (strncmp(argv[i],"--deep=",7)==0) {
            recur_deep = atoi(argv[i]+7);
        }
        else if (strcmp(argv[i],"--lnk")==0)
            _args[ARGS_LINK] = 1;
        else if (strcmp(argv[i],"--path")==0)
            _args[ARGS_PATH] = 1;
        else if (strcmp(argv[i], "--show-stats") == 0) {
            _args[ARGS_SHOWSTATS] = 1;
            _args[ARGS_STATIS] = 1;
        }
        else if (strcmp(argv[i],"--color")==0)
            _args[ARGS_COLOR] = 1;
        else if (strcmp(argv[i], "--create-time") == 0)
            _args[ARGS_CREATTM] = 1;
        else if (strcmp(argv[i],"--sort")==0)
            _args[ARGS_SORT] = SORT_BYNAME;
        else if (strncmp(argv[i],"--sort=",7)==0) {
            if (strlen(argv[i])==8) {
                if (argv[i][7] == SORT_BYTM
                    || argv[i][7] == SORT_BYCHTM
                    || argv[i][7] == SORT_BYSIZE
                    || argv[i][7] == SORT_BYNAME
                ){
                    _args[ARGS_SORT] = argv[i][7];
                } else {
                    _args[ARGS_SORT] = SORT_BYNAME;
                }
            } else {
                dprintf(2, "Error:unknow sort type\n");
                return 1;
            }
        }
        else if (strcmp(argv[i],"--regex")==0) {
            i++;
            if (i >= argc) {
                dprintf(2,"Error:less argument -> --regex [REGEX]");
                return 2;
            }
            if (regcomp(_regcom, argv[i], REG_EXTENDED|REG_ICASE)!=0) {
                dprintf(2, "Error: compile regex -> %s\n",argv[i]);
                perror("regcomp");
                return 2;
            }
            _args[ARGS_REGEX] = 1;
        }
        else if (strcmp(argv[i], "-h")==0) {
            help();
            return 0;
        }
        else if (argv[i][0]=='-') {
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
                else if (argv[i][a] == 't')
                    _args[ARGS_STATIS] = 1;
                else if (argv[i][a] == 'f')
                    _args[ARGS_TYPE] = 1;
                else if (argv[i][a] == 'l')
                    _args[ARGS_LONGINFO] = 1;
                else if (argv[i][a] == 'r') {
                    _args[ARGS_RECUR] = 1;
                    recur_deep = 0;
                }
                else if (argv[i][a] == 'c')
                    _args[ARGS_CREATTM] = 1;
            }
        }
        else if (access(argv[i], F_OK)==0) {
            len_buf = strlen(argv[i]);
            if (len_buf >= MAX_NAME_LEN) {
                dprintf(2,"Error: the length of path too long -> %s\n",argv[i]);
                continue;
            }
            if (len_buf > 1 && argv[i][len_buf-1]=='/') {
                strncpy(_path_buffer, argv[i], len_buf-1);
                _path_buffer[len_buf-1] = '\0';
                init_path_list(_path_buffer, &pl);
            } else {
                init_path_list(argv[i], &pl);
            }
        }
        else
            dprintf(2, "Error:unknow arguments -> %s\n", argv[i]);
    }
    if (pl.next==NULL)
        init_path_list(".",&pl);
    
    recur_dir(pl.next, recur_deep, &stats);

    if (_args[ARGS_STATIS])
        out_statis(&stats);
    
    destroy_path_list(pl.next);

    return 0;
}

int init_path_list(char* path, struct path_list* pl) {
    struct stat pst;
    if (lstat(path, &pst) < 0) {
        dprintf(2, "Error:init path list -> %s\n",path);
        perror("lstat");
        return -1;
    }
    struct path_list * ptmp = NULL;
    ptmp = (struct path_list*)malloc(sizeof(struct path_list));
    if (ptmp == NULL) {
        perror("malloc");
        return -1;
    }
    if (S_ISDIR(pst.st_mode)) {
        ptmp->is_dir = 1;
    } else {
        ptmp->is_dir = 0;
    }
    struct path_list * end_path = pl;
    while (end_path!=NULL && end_path->next!=NULL) {
        end_path = end_path->next;
    }

    end_path->next = ptmp;
    strcpy(ptmp->path, path);
    ptmp->next = NULL;
    end_path = ptmp;
    ptmp = NULL;

    return 0;
}

int recur_dir(struct path_list* pl, int max_height, struct statis* stats){
    struct path_list* cur = pl;
    struct path_list * path_tmp = NULL;
    struct path_list * end_path = pl;
    struct file_buf_list * fbuf_tmp = NULL;
    
    while (end_path!=NULL && end_path->next!=NULL) {
        end_path = end_path->next;
    }

    DIR* d = NULL;
    struct dirent * rd = NULL;
    struct stat sttmp;
    char nbuf[MAX_NAME_LEN] = {'\0'};
    int cur_files_count = 0;
    int cur_name_max_len = 0;
    int len_buf = 0;

    struct passwd * pd; 
    struct group * grp;
    struct format_info fi;
    struct file_buf * fbtmp = NULL;
    struct file_buf one_buf;

    bzero(&fi, sizeof(fi));

    while (cur!=NULL) {
        if(max_height>0 && cur->height >= max_height)break;
        if (cur->is_dir==0) {
            if (lstat(cur->path, &one_buf.st) < 0){
                dprintf(2,"Error: %s\n",cur->path);
                perror("lstat");
            } else {
                start_statis(one_buf.st, stats);
                strcpy(one_buf.name, cur->path);
                one_buf.path[0] = '\0';
                pd = getpwuid(sttmp.st_uid);
                grp = getgrgid(sttmp.st_gid);
                strcpy(one_buf.uname, pd->pw_name);
                strcpy(one_buf.group, grp->gr_name);
                
                fi.uname_max_len = strlen(pd->pw_name);
                fi.name_max_len = strlen(cur->path);
                fi.group_max_len = strlen(grp->gr_name);

                out_info(&one_buf, fi);
            }
            goto out_next;
        }
        d = opendir(cur->path);
        if (d==NULL) {
            perror("opendir");
            cur = cur->next;
            continue;
        }
        
        int cur_count = 0;
        while((rd = readdir(d))!=NULL) {
            strcpy(nbuf, cur->path);
            strcat(nbuf, "/");
            if (strlen(rd->d_name)+strlen(nbuf)>MAX_NAME_LEN) {
                continue;
            }

            if (_args[ARGS_LSALL]==0) {
                if (strcmp("..", rd->d_name)==0 || strcmp(".", rd->d_name)==0)
                    continue;
                else if (rd->d_name[0]=='.')
                    continue;
            }

            strcat(nbuf, rd->d_name);
            if (lstat(nbuf, &sttmp)==-1)
                return -1;
            pd = getpwuid(sttmp.st_uid);
            grp = getgrgid(sttmp.st_gid);

            if (S_ISDIR(sttmp.st_mode) 
                && strcmp(rd->d_name, "..")!=0
                && strcmp(rd->d_name, ".")!=0
            ) {
                path_tmp = (struct path_list*)malloc(sizeof(struct path_list));
                if (path_tmp==NULL)
                    return -1;
                path_tmp->is_dir = 1;
                path_tmp->next = NULL;

                path_tmp->height = cur->height+1;
                strcpy(path_tmp->path, nbuf);
                
                end_path->next = path_tmp;
                end_path = path_tmp;
                path_tmp = NULL;
            }

            if (_args[ARGS_REGEX] && !S_ISDIR(sttmp.st_mode)) {
                if (regexec(_regcom, rd->d_name, 1, _regmatch, 0)!=0)
                    continue;
                cur_count++;
            }
            if (_args[ARGS_STATIS] || _args[ARGS_SHOWSTATS])
                start_statis(sttmp, stats);

            if (_file_cache.end_ind < BUFF_LEN) {
                _file_cache.end_ind += 1;
                fbtmp = &_file_cache.fcache[_file_cache.end_ind];
            } else {
                fbuf_tmp = (struct file_buf_list*)malloc(sizeof(struct file_buf_list));
                if (fbuf_tmp == NULL) {
                    perror("malloc");
                    return -1;
                }
                fbuf_tmp->next = NULL;
                fbuf_tmp->count = _fbuf_list_end->count + 1;
                _fbuf_list_end->next = fbuf_tmp;
                _fbuf_list_end = fbuf_tmp;
                fbuf_tmp = NULL;
                fbtmp = &_fbuf_list_end->fbuf;
            }
            
            memcpy(&fbtmp->st, &sttmp, sizeof(struct stat));
            strcpy(fbtmp->path, nbuf);
            strcpy(fbtmp->name, rd->d_name);
            strcpy(fbtmp->uname, pd->pw_name);
            strcpy(fbtmp->group, grp->gr_name);

            fi.count++;
            len_buf = strlen(rd->d_name);
            if (len_buf > fi.name_max_len)
                fi.name_max_len = len_buf;

            len_buf = strlen(pd->pw_name);
            if (len_buf > fi.uname_max_len)
                fi.uname_max_len = len_buf;

            len_buf = strlen(grp->gr_name);
            if (len_buf > fi.group_max_len)
                fi.group_max_len = len_buf;
        }
        if ((_args[ARGS_REGEX] && cur_count > 0) || _args[ARGS_REGEX]==0) {
            if (out_control(cur->path, fi) < 0) {
                return -1;
            }
        }
        cur_count = 0;
        bzero(&fi, sizeof(fi));
        closedir(d);
      out_next:;
        cur = cur->next;
    } 
}

void start_statis(struct stat sttmp, struct statis * stats) {
    stats->total_count++;
    stats->total_size += sttmp.st_size;
    
    if (!S_ISDIR(sttmp.st_mode))
        stats->file_total_size += sttmp.st_size;

    if (S_ISDIR(sttmp.st_mode))
        stats->dir_count++;
    else if (S_ISREG(sttmp.st_mode))
        stats->file_count++;
    else if (S_ISLNK(sttmp.st_mode))
        stats->link_count++;
    else if (S_ISFIFO(sttmp.st_mode))
        stats->fifo_count++;
    else if (S_ISSOCK(sttmp.st_mode))
        stats->sock_count++;
    else if (S_ISCHR(sttmp.st_mode))
        stats->chr_count++;
    else if (S_ISBLK(sttmp.st_mode))
        stats->blk_count++;
}


int out_control(char *path, struct format_info fi) {
    if (_args[ARGS_SHOWSTATS]) {
        return 0;
    }

    if (_args[ARGS_PATH]==0
        && _args[ARGS_SHOWSTATS]==0 
        && _args[ARGS_REGEX]==0
    ) {
        printf("%s/:\n", path);
    }

    struct file_buf_list * fbuf = _fbuf_list_head.next;
    int total = _file_cache.end_ind + 1 + _fbuf_list_end->count;
    struct file_buf ** fbufs = (struct file_buf**)malloc(sizeof(struct file_buf*)*total);
    if (fbufs==NULL) {
        perror("malloc");
        destroy_file_list(_fbuf_list_head.next);
        return -1;
    }

    int i=0;
    for (i=0; i<= _file_cache.end_ind; i++)
        fbufs[i] = _file_cache.fcache+i;
    while(fbuf!=NULL) {
        fbufs[i++] = &fbuf->fbuf;
        fbuf = fbuf->next;
    }

    qsortfbuf(fbufs,0,total-1);
    for (i=0;i<total;i++)
        out_info(fbufs[i], fi);

    if (_args[ARGS_PATH]==0)printf("\n");

    _file_cache.end_ind = -1;
    destroy_file_list(_fbuf_list_head.next);
    reset_file_buf_list();
    free(fbufs);
    fbufs = NULL;
    return 0;
}

void out_info(struct file_buf * fb, struct format_info fi) {
    char align_space[MAX_NAME_LEN];
    for(int i=0;i<MAX_NAME_LEN;i++)align_space[i] = ' ';
    align_space[MAX_NAME_LEN-1] = '\0';

    int align_i = 0;

    if (_args[ARGS_REGEX] && S_ISDIR(fb->st.st_mode))
        return ;

    if (_args[ARGS_INO])
        printf("%-9d ", fb->st.st_ino);
    
    if(_args[ARGS_MODE] || _args[ARGS_LONGINFO])
        printf("%o ", fb->st.st_mode & 0777);
    
    if (_args[ARGS_LONGINFO]) {
        printf("%d ", fb->st.st_nlink);

        align_i = fi.uname_max_len - strlen(fb->uname) + 1;
        align_space[align_i] = '\0';
        printf("%s%s",fb->uname,align_space);
        
        align_space[align_i] = ' ';
        align_i = fi.group_max_len - strlen(fb->group) + 1;
        align_space[align_i] = '\0';
        printf("%s%s",fb->group, align_space);
        align_space[align_i] = ' ';

    }

    if (_args[ARGS_CREATTM]) {
        char create_time[64] = {'\0'};
        strcpy(create_time, ctime(&fb->st.st_mtim.tv_sec));
        create_time[strlen(create_time)-1] = '\0';
        printf("%s ",create_time);
    }
    
    if ((_args[ARGS_PATH] || _args[ARGS_REGEX]) && strlen(fb->path)>0)
        printf("%s",fb->path);
    else {
        if (_args[ARGS_COLOR]) {
            if (S_ISDIR(fb->st.st_mode))
                printf("\e[1;34m%s",fb->name);
            else if(S_ISLNK(fb->st.st_mode))
                printf("\e[2;36m%s",fb->name);
            else if (S_ISREG(fb->st.st_mode) && access(fb->path,X_OK)==0)
                printf("\e[1;35m%s",fb->name);
            else
                printf("%s",fb->name);
            printf("\e[0;m");
        }
        else
            printf("%s", fb->name);
    }
    
    char flag = '\0';
    if (_args[ARGS_TYPE]) {
        if (S_ISDIR(fb->st.st_mode))
            flag = TYPE_DIR;
        else if (S_ISLNK(fb->st.st_mode))
            flag = TYPE_LNK;
        else if (S_ISFIFO(fb->st.st_mode))
            flag = TYPE_FIFO;
        else if (S_ISSOCK(fb->st.st_mode))
            flag = TYPE_SOCK;
        else if (S_ISCHR(fb->st.st_mode))
            flag = TYPE_CHR;
        else if (S_ISBLK(fb->st.st_mode))
            flag = TYPE_BLK;
        else if (S_ISREG(fb->st.st_mode) && access(fb->path,X_OK)==0)
            flag = TYPE_EXEC;
        if (flag > 0)
            printf("%c",flag);
    }

    align_i = fi.name_max_len - strlen(fb->name) + 1 + (flag>0?0:1);
    align_space[align_i] = '\0';
    printf("%s", align_space);

    if (_args[ARGS_SIZE] || _args[ARGS_LONGINFO]) {
        if (fb->st.st_size <= 1024)
            printf(" %dB",fb->st.st_size);
        else if (fb->st.st_size > 1024 && fb->st.st_size < 1048576)
            printf(" %.2lfK", (double)fb->st.st_size/1024);
        else
            printf(" %.2lfM",(double)fb->st.st_size/1048576);
    }
    
    if (S_ISLNK(fb->st.st_mode) 
        && (_args[ARGS_LONGINFO] || _args[ARGS_LINK])
    ) {
        char _path_buffer[MAX_NAME_LEN];
        int link_len = readlink(fb->path, _path_buffer, MAX_NAME_LEN-1);
        if (link_len > 0) {
            _path_buffer[link_len] = '\0';
            printf("-> %s ", _path_buffer);
        }
    }

    printf("\n");
}
void format_size(unsigned int size, char * fstr) {
    if (size <= 1024)
        sprintf(fstr, "%dB",size);
    else if (size > 1024 && size < 1048576)
        sprintf(fstr, "%.2lfK", (double)size/1024);
    else
        sprintf(fstr, "%.2lfM", (double)size/1048576);
}

void out_statis(struct statis * stats) {
    printf("-- statis --\n");
    char * count_name[] = {
        "regular file",
        "fifo file",
        "link file",
        "sock file",
        "directory",
        "char device",
        "block device",
        "\0"
    };

    unsigned long long count_ind[] = {
        stats->file_count, stats->fifo_count, stats->link_count, 
        stats->sock_count, stats->dir_count, stats->chr_count,
        stats->blk_count
    };
    int i=0;
    while (strcmp(count_name[i],"\0")!=0) {
        if (count_ind[i]>0)
            printf("%13s : %ld\n", count_name[i], count_ind[i]);
        i++;
    }
    printf("%13s : %ld\n", "total count",stats->total_count);
    
    char size_str[64] = {'\0',};
    format_size(stats->total_size, size_str);
    size_str[31] = '\0';
    format_size(stats->file_total_size, size_str+32);
    size_str[63] = '\0';

    printf("%13s : %s\n%13s : %s\n", "total size",size_str,"total file size",size_str+32);
}

void destroy_file_list(struct file_buf_list * fl) {
    struct file_buf_list * ftmp = fl;
    while(fl!=NULL) {
        ftmp = fl->next;
        free(fl);
        fl = ftmp;
    }
}

void reset_file_buf_list() {
    _fbuf_list_head.next = NULL;
    _fbuf_list_head.count = 0;
    _fbuf_list_end = &_fbuf_list_head;
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

