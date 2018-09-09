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
#include <sys/ioctl.h>
#include <termios.h>

#define PROGRAM_NAME    "li"

#define TYPE_DIR        '/'
#define TYPE_LNK        '@'
#define TYPE_FIFO       '|'
#define TYPE_SOCK       '='
#define TYPE_EXEC       '*'
#define TYPE_CHR        '%'
#define TYPE_BLK        '#'

#define TYPE_INFO   "\
    / : dir\n\
    @ : symble link\n\
    | : FIFO(PIPE)\n\
    = : socket\n\
    * : exec\n\
    %% : character device\n\
    # : block device\n"

void list_type_info() {
    printf(TYPE_INFO);
}

#define SORT_BYNAME     0
#define SORT_BYTM       1
#define SORT_BYCHTM     2
#define SORT_BYSIZE     3

#define SORT_TY_CELL    'd'
#define SORT_TY_ALL     'a'


#define ARGS_PATH       0
#define ARGS_SIZE       1
#define ARGS_LINK       2
#define ARGS_TYPE       3    //char for file type
#define ARGS_MODE       4
#define ARGS_USRGRP     5
#define ARGS_CREATM     6  //create time
#define ARGS_CHANGTM    7  //last change time
#define ARGS_ACCESTM    8    //last access time
#define ARGS_REGEX      9 
#define ARGS_RECUR      10
#define ARGS_STATIS     11    //recursion
#define ARGS_DIRSELF    12    //statistic
#define ARGS_REGNODIR   13
#define ARGS_REGNOFIL   14
#define ARGS_SORT       15
#define ARGS_BLOCK      16
#define ARGS_NODIR      17    //not list dir, just file
#define ARGS_SORTYP     18    //sort type : sort in cell of dir or sort all fil
#define ARGS_ONELINE    19    //show info in one line
#define ARGS_LSALL      20
#define ARGS_INO        21
#define ARGS_LONGINFO   22
#define ARGS_SHOWSTATS  23
#define ARGS_COLOR      24
#define ARGS_OUTMORE    25

#define ARGS_END        32

#define STDOUT_SCRN     1
#define STDOUT_FIFO     2
#define STDOUT_FILE     3

#define MAX_NAME_LEN    2048
#define PATH_CELL_END   16


/*
    这两个函数是简单的修复函数，用于输出格式的时候对齐，
    因为默认的UTF8编码使用变长字节表示字符，但是终端显示，
    汉字占用两个英文字母的位置。
    在解决这个问题时，我还没想到完美的办法。
    Linux默认使用UTF8编码，常用汉字以及标点字符占用3字节。
    而占用3字节的汉字大概有20000个，基本涵盖了所有常用字。

    我的处理办法是检测是否存在中文字符，存在的话则计算出
    有多少个中文字符，除以3然后乘以2计算真实的输出长度。
*/

int is_chinese_name(char *name, int len) {
    for(int i=0; i<len; i++) {
        if (name[i] < 0) {
            return 1;
        }
    }
    return 0;
}

int name_out_len(char *name, int len) {
    int out_len = 0;
    int chinese_len = 0;
    for(int i=0; i<len; i++) {
        if (name[i]<0) {
            chinese_len += 1;
        }
    }
    
    out_len = (len - chinese_len ) + ((chinese_len/3)*2);
    return out_len;
}

int true_out_len (char *name) {
    int len = strlen(name);
    int olen = 0;
    if (is_chinese_name(name, len)) {
        olen = name_out_len(name, len);
        //printf("true:%d  out:%d\n", len, olen);
        return olen;
    }
    return len;
}

/*
    save path info 
*/
struct path_cell {
    char path[MAX_NAME_LEN];
    char is_root;
    int  plen;
    int  height;
};

/*
    path list for recur dir 
*/
struct path_list {
    struct path_cell pce[PATH_CELL_END];
    int end_ind;

    struct path_list * next;
    struct path_list * prev;
    struct path_list * plast;
};

struct path_list *
init_path_list(struct path_list* pl) {
   pl->end_ind = 0;
   pl->next = NULL;
   pl->prev = NULL;
   pl->plast = pl;
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
add_path_list(struct path_list * pl, char * path, int height);

struct path_list *
del_path_list(struct path_list * pl, struct path_list * pnode);

struct path_list *
get_path_list_last(struct path_list * pl);


struct path_list *
add_path_list(struct path_list * pl, char * path, int height) {
    struct path_list * plast =  pl->plast; //get_path_list_last(pl);
    struct path_cell * pcell = NULL;
    struct path_list * ptmp;

    //printf("add path: %s\n", path);
    if (plast->end_ind < PATH_CELL_END) {
        pcell = plast->pce+plast->end_ind;
        plast->end_ind += 1;
    } else {
        ptmp = (struct path_list*)malloc(sizeof(struct path_list));
        if (ptmp==NULL) {
            perror("malloc");
            return NULL;
        }
        plast->next = ptmp;
        ptmp->next = NULL;
        ptmp->prev = plast;
        ptmp->end_ind = 0;
        pcell = ptmp->pce;
        pl->plast = ptmp;
    }

    strcpy(pcell->path, path);
    pcell->is_root = 0;
    int path_len = strlen(path);
    if (path_len==1 && path[0]=='/') {
        pcell->is_root = 1;
    }

    pcell->plen = path_len;
    if (height > 0) {
        pcell->height = height;
    }

    return ptmp;
}

struct path_list *
del_path_list(struct path_list * pl, struct path_list * pnode) {
    
}

struct path_list *
get_path_list_last(struct path_list * pl) {
    struct path_list * pcur = pl;
    while (pcur!=NULL && pcur->next!=NULL) {
        pcur = pcur->next;
    }
    return pcur;
}

/*
    global vars
*/
struct infoargs {

    //main arguments
    char args[ARGS_END];

    //for regex
    regex_t    regcom[1];
    regmatch_t regmatch[1];
    
    //standard out type
    int stdout_type;

    int out_dir_flag;
};

int set_stdout_type(int * stdo) {
    struct stat dst;
    if (fstat(1, &dst) < 0) {
        perror("fstat");
        return -1;
    }

    if (S_ISFIFO(dst.st_mode))
        *stdo = STDOUT_FIFO;
    else if (S_ISCHR(dst.st_mode))
        *stdo = STDOUT_SCRN;
    else
        *stdo = STDOUT_FILE;

    return 0;
}

void init_infoargs(struct infoargs * ia) {
    bzero(ia->args, sizeof(char)*ARGS_END);
    ia->stdout_type = STDOUT_SCRN;
    set_stdout_type(&ia->stdout_type);
    ia->out_dir_flag = 1;
}

//define global var
struct infoargs _iargs;



/*
   定义文件缓存的结构，用于存储获取到的文件名、
   状态信息、所属用户和组等信息。
   然后定义文件缓存的链表结构，定义文件列表缓存结构。
   文件列表缓存结构包含文件缓存结构体数组，以及文件缓存链表
   当数组使用完毕开始使用链表，目的在于平衡存储与性能。
*/
struct file_buf {
    char name[256];
    char path[MAX_NAME_LEN];
    int name_len;
    int path_len;

    int name_is_ch;
    int path_is_ch;
    int name_outlen;
    int path_outlen;
    
    char uname[40];
    char group[40];
    int height;
    int dir_is_hide; //如果是正则表达式搜索模式，用于标记是否显示，用于目录
    struct stat st;
};

#define BUFF_LEN    4096

struct file_buf_list {
    struct file_buf fbuf;
    struct file_buf_list * next;
};

struct file_list_cache {
    int end_ind;
    struct file_buf fcache[BUFF_LEN+1];
    int list_count;
    struct file_buf_list fbhead;
    struct file_buf_list * flast;

    //for sort
    struct file_buf **fba;
};

struct format_info {
    int count;
    int name_max_len;
    int uname_max_len;
    int group_max_len;
    int win_row;
    int win_col;
    int out_row;

    int max_ino_bits;
    int max_nlink_bits;
};

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

void init_flist_cache (struct file_list_cache * flcache) {
    flcache->end_ind = 0;
    flcache->list_count = 0;
    flcache->fbhead.next = NULL;
    flcache->flast = &flcache->fbhead;
    flcache->fba = NULL;
}

int set_st_fbuf(struct file_buf *fbuf, 
        struct stat *st, char * name, char *path, int height)
{
    memcpy(&fbuf->st, st, sizeof(struct stat));
    if (name) {
        strcpy(fbuf->name, name);
        fbuf->name_len = strlen(name);
        fbuf->name_is_ch = is_chinese_name(name, fbuf->name_len);
        fbuf->name_outlen = name_out_len(name, fbuf->name_len);
    }

    if (path){
        strcpy(fbuf->path, path);
        fbuf->path_len = strlen(path);
        fbuf->path_is_ch = is_chinese_name(path, fbuf->path_len);
        fbuf->path_outlen = name_out_len(path, fbuf->path_len);
    }

    if (height > 0)
        fbuf->height = height;

    fbuf->uname[0] = '\0';
    fbuf->group[0] = '\0';

    struct passwd * pd; 
    struct group * grp;
    pd = getpwuid(fbuf->st.st_uid);
    grp = getgrgid(fbuf->st.st_gid);
    if (pd && grp) {
        strcpy(fbuf->uname, pd->pw_name);
        strcpy(fbuf->group, grp->gr_name);
    }
    fbuf->dir_is_hide = 0;
    
    return 0;
}

void set_fbuf_hide(struct file_buf *fbuf, int hide) {
    fbuf->dir_is_hide = hide;
}


int add_to_flcache(struct file_list_cache * flcache, struct file_buf * fbuf) {
    struct file_buf * fbtmp = NULL;
    struct file_buf_list *fl = NULL;
    if (flcache->end_ind < BUFF_LEN) {
        fbtmp = flcache->fcache+flcache->end_ind;
        flcache->end_ind += 1;

    } else {
        fl = (struct file_buf_list *)malloc(sizeof(struct file_buf_list));
        if (fl == NULL) {
            perror("malloc");
            return -1;
        }
        fl->next = NULL;
        flcache->flast->next = fl;
        flcache->flast = fl;
        flcache->list_count += 1;
        fbtmp = &fl->fbuf;
    }
    memcpy(fbtmp, fbuf, sizeof(struct file_buf));

    return 0;
}

void destroy_flcache(struct file_list_cache *flcache) {
    flcache->end_ind = 0;
    struct file_buf_list * fbtmp = flcache->fbhead.next;
    struct file_buf_list * fbtmp2 = NULL;
    
    while(fbtmp) {
        fbtmp2 = fbtmp->next;
        free(fbtmp);
        fbtmp = fbtmp2;
    }
    flcache->list_count = 0;
    flcache->fbhead.next = NULL;
    flcache->flast = &flcache->fbhead;
    free(flcache->fba);
    flcache->fba = NULL;
}


int fbuf_sort(struct file_list_cache * flcache, int sort_flag) {
    int total_count = flcache->end_ind + flcache->list_count;
    flcache->fba = (struct file_buf**)malloc(sizeof(struct file_buf*)*total_count);
    if (flcache->fba==NULL) {
        return -1;
    }

    int i=0;
    for (; i<flcache->end_ind; i++)
        flcache->fba[i] = flcache->fcache+i;

    struct file_buf_list *fl = flcache->fbhead.next;
    while(fl) {
        flcache->fba[i] = &fl->fbuf;
        i++;
        fl = fl->next;
    }
    
    qsortfbuf(flcache->fba, 0, total_count-1);

    return 0;
}


int
add_fbuf_dirs_to_plist(struct file_list_cache* flcache, 
        struct path_list* plist)
{

    struct file_buf *fbtmp = NULL;
    struct file_buf_list * fl;
    int i=0;
    int total = flcache->list_count + flcache->end_ind;
    for(i=0; i<total; i++) {
        if (S_ISDIR(flcache->fba[i]->st.st_mode))
            add_path_list(plist, flcache->fba[i]->path, flcache->fba[i]->height);
    }

    return 0;
}


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


struct allinfocell {
    struct statis stats;
    struct format_info fi;
    struct file_list_cache flcache;
};

struct allinfocell _aic;
struct path_list _pathlist;

#define MAX_OUTLINE_LEN     4096

void out_color(struct file_buf * fb, char *pname) {
    if (S_ISDIR(fb->st.st_mode))
        printf("\e[1;34m%s",pname);
    else if(S_ISLNK(fb->st.st_mode))
        printf("\e[1;35m%s",pname);
    else if (S_ISREG(fb->st.st_mode) && access(fb->path,X_OK)==0)
        printf("\e[2;36m%s",pname);
    else if (S_ISFIFO(fb->st.st_mode))
        printf("\e[2;33m%s", pname);
    else if (S_ISCHR(fb->st.st_mode) || S_ISBLK(fb->st.st_mode))
        printf("\e[2;31m%s", pname);
    else if (S_ISSOCK(fb->st.st_mode))
        printf("\e[2;35m%s", pname);
    else
        printf("%s", pname);
    printf("\e[0;m");
}

void format_size(unsigned long long size, char * fstr) {
    if (size <= 1024)
        sprintf(fstr, "%lluB",size);
    else if (size > 1024 && size < 1048576)
        sprintf(fstr, "%.2lfK", (double)size/1024);
    else
        sprintf(fstr, "%.2lfM", (double)size/1048576);
}


void out_info(struct file_buf *fbuf, char *ppath, 
        struct format_info *fi, char *outline)
{
    char fmt_str[256] = {'\0'};
    int posi = 0, count=0;
    
    char * week [] = {
        "Mon", "Tue", "Wed", "Tur", "Fri", "Sat", "Sun"
    };

    if (_iargs.args[ARGS_INO]) {
        sprintf(fmt_str, "%%-%ulu ", fi->max_ino_bits+1);
        count = sprintf(outline + posi, fmt_str, fbuf->st.st_ino);
        posi += count;
    }
    
    if (_iargs.args[ARGS_MODE] || _iargs.args[ARGS_LONGINFO]) {
        count = sprintf(outline + posi, "%o ", fbuf->st.st_mode & 0777);
        posi += count;
    }

    if (_iargs.args[ARGS_LONGINFO]) {
        sprintf(fmt_str, "%%-%ulu ", fi->max_nlink_bits+1);
        count = sprintf(outline + posi, fmt_str, fbuf->st.st_nlink);
        posi += count;
    }
    
    if (_iargs.args[ARGS_USRGRP] || _iargs.args[ARGS_LONGINFO]) {
        sprintf(fmt_str, "%%-%ds %%-%ds ", fi->uname_max_len, fi->group_max_len);
        count = sprintf(outline + posi, fmt_str, fbuf->uname, fbuf->group);
        posi += count;
    }

    if (_iargs.args[ARGS_CREATM]) {
        time_t t = time(NULL);
        struct tm *ct = localtime(&t);
        count = sprintf(outline+posi, "%d.%02d.%02d %02d:%02d %s ", 
                ct->tm_year+1900, ct->tm_mon+1, ct->tm_mday, 
                ct->tm_hour, ct->tm_min, week[ct->tm_wday]);
        posi += count;
    }

    
    int fmt_name_len = 0;
    if ((_iargs.args[ARGS_PATH] || _iargs.args[ARGS_REGEX]) 
        && ppath
    ) {

        count = sprintf(outline + posi, "%s/", ppath);
        posi += count;
    }

    fmt_name_len = fi->name_max_len;
    
    count = sprintf(outline+posi, "%s", fbuf->name);
    posi += count;

    char flag = '\0';
    if (_iargs.args[ARGS_TYPE]) {
        fmt_name_len += 1;
        if (S_ISDIR(fbuf->st.st_mode))
            flag = TYPE_DIR;
        else if (S_ISLNK(fbuf->st.st_mode))
            flag = TYPE_LNK;
        else if (S_ISFIFO(fbuf->st.st_mode))
            flag = TYPE_FIFO;
        else if (S_ISSOCK(fbuf->st.st_mode))
            flag = TYPE_SOCK;
        else if (S_ISCHR(fbuf->st.st_mode))
            flag = TYPE_CHR;
        else if (S_ISBLK(fbuf->st.st_mode))
            flag = TYPE_BLK;
        else if (S_ISREG(fbuf->st.st_mode) && access(fbuf->path,X_OK)==0)
            flag = TYPE_EXEC;
        if (flag =='\0'){
            flag = ' ';
        }
        count = sprintf(outline+posi, "%c", flag);
        posi += count;
    }
    
    if (_iargs.args[ARGS_OUTMORE]) {
        unsigned long int out_len = fmt_name_len - 
                (fbuf->name_is_ch ? fbuf->name_outlen : fbuf->name_len);
        sprintf(fmt_str, "%%-%luc", out_len);
        count = sprintf(outline+posi, fmt_str, ' ');
        posi += count;
    }
    
    count = sprintf(outline+posi, " ");
    posi += count;

    if (_iargs.args[ARGS_SIZE] || _iargs.args[ARGS_LONGINFO]) {
        format_size(fbuf->st.st_size, outline+posi);
        posi = strlen(outline);
    }

    if (_iargs.args[ARGS_LINK] || _iargs.args[ARGS_LONGINFO]) {
        
        char _path_buffer[MAX_NAME_LEN];
        int link_len = readlink(fbuf->path, _path_buffer, MAX_NAME_LEN-1);
        if (link_len > 0) {
            _path_buffer[link_len] = '\0';
            count = sprintf(outline+posi, " -> %s", _path_buffer);
            posi += count;
        }
    }

}


int out_flcache(struct file_list_cache *flcache, 
        char *path, struct format_info *fi)
{

    char outline[MAX_OUTLINE_LEN+128];
    char outcolor[MAX_OUTLINE_LEN+128];
    int max_out_len = strlen(path)
                    + fi->uname_max_len
                    + fi->group_max_len
                    + fi->name_max_len;
    if (max_out_len > MAX_OUTLINE_LEN) {
        return -1;
    }

    if (fbuf_sort(flcache, SORT_BYNAME)<0) {
        return -1;
    }

    if (!_iargs.args[ARGS_PATH]
        && !_iargs.args[ARGS_REGEX]
        && !_iargs.args[ARGS_SHOWSTATS]
        && _iargs.out_dir_flag
    ) {
        printf("%s/:\n", path);
    }

    int max_len = fi->name_max_len + 2;
    
    if (_iargs.args[ARGS_TYPE])
        max_len += 1;

    fi->out_row = fi->win_col/max_len;

    int fmt_width = fi->win_col/fi->out_row;
    
    char fmt_str[128] = {'\0'};
    int next_line = 0;
    int total = flcache->end_ind + flcache->list_count;
    
    int true_len = 0;
    int name_len = 0;
    for(int i=0; i<total; i++) {
        
        if (S_ISDIR(flcache->fba[i]->st.st_mode)
            && flcache->fba[i]->dir_is_hide
        ) {
            continue;
        }
        name_len = flcache->fba[i]->name_len;
        true_len = flcache->fba[i]->name_outlen;

        out_info(flcache->fba[i], path, fi, outline);
        
        if (_iargs.args[ARGS_OUTMORE]) {
            if (_iargs.args[ARGS_COLOR] 
                && _iargs.stdout_type == STDOUT_SCRN
            ) {
                out_color(flcache->fba[i], outline);
                printf("\n");
            }
            else
                printf("%s\n", outline);
        } else {
            sprintf(fmt_str, "%%-%ds", max_len+name_len-true_len);
            if (_iargs.args[ARGS_COLOR]
                && _iargs.stdout_type == STDOUT_SCRN
            ) {
                sprintf(outcolor, fmt_str, outline);
                out_color(flcache->fba[i], outcolor);
            } else {
                printf(fmt_str, outline);
            }
            next_line += 1;
            if (next_line >= fi->out_row) {
                next_line = 0;
                printf("\n");
            }
        }
    }
    if (next_line!=0) {
        printf("\n");
    }

    return 0;
}

void start_statis(struct stat *sttmp, struct statis * stats) {
    stats->total_count++;
    stats->total_size += sttmp->st_size;
    
    if (!S_ISDIR(sttmp->st_mode))
        stats->file_total_size += sttmp->st_size;

    if (S_ISDIR(sttmp->st_mode))
        stats->dir_count++;
    else if (S_ISREG(sttmp->st_mode))
        stats->file_count++;
    else if (S_ISLNK(sttmp->st_mode))
        stats->link_count++;
    else if (S_ISFIFO(sttmp->st_mode))
        stats->fifo_count++;
    else if (S_ISSOCK(sttmp->st_mode))
        stats->sock_count++;
    else if (S_ISCHR(sttmp->st_mode))
        stats->chr_count++;
    else if (S_ISBLK(sttmp->st_mode))
        stats->blk_count++;
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
            printf("%13s : %llu\n", count_name[i], count_ind[i]);
        i++;
    }

    
    char size_str[64] = {'\0',};
    format_size(stats->total_size, size_str);
    size_str[31] = '\0';
    format_size(stats->file_total_size, size_str+32);
    size_str[63] = '\0';

    if (!_iargs.args[ARGS_REGEX]) {
        printf("%13s : %llu\n", "total count",stats->total_count);
        printf("%13s : %s\n", "total size",size_str);
    }
    printf("%13s : %s\n","total file size",size_str+32);
}

void caclt_fi(struct file_buf * fbuf, struct format_info *fi, struct stat *st) {
    
    unsigned long len_buf;

    len_buf = (fbuf->name_is_ch ? fbuf->name_outlen : fbuf->name_len);
    if (fi->name_max_len < len_buf)
        fi->name_max_len = len_buf;

    len_buf = strlen(fbuf->uname);
    if (fi->uname_max_len < len_buf)
        fi->uname_max_len = len_buf;

    len_buf = strlen(fbuf->group);
    if (fi->group_max_len < len_buf)
        fi->group_max_len = len_buf;

    len_buf = (unsigned long)log10((double)st->st_ino);
    if (fi->max_ino_bits < len_buf)
        fi->max_ino_bits = len_buf;

    len_buf = (unsigned long)log10((double)st->st_nlink);
    if (fi->max_nlink_bits < len_buf)
        fi->max_nlink_bits = len_buf;
}

#define LINUX_NAME_LEN      256

int recur_dir(int deep) {
    struct path_list *pl = &_pathlist;
    struct path_cell *pcell = NULL;
    struct stat stmp;
    struct file_buf fbuf;

    DIR * d = NULL;
    struct dirent *rd = NULL;

    int i,k;
    int cur_height = 1;

    char pathbuf[MAX_NAME_LEN+1] = {'\0'};
    unsigned long int len_buf = 0;
    int regex_count = 0;
    int stats_flag = 0;
    while (pl) {
 
        for(i=0; i<pl->end_ind; i++) {
            pcell = &pl->pce[i];
            cur_height = pcell->height;
            if (deep > 0 && cur_height > deep)goto end_recur;

            d = opendir(pcell->path);
            if (!d) {
                perror("opendir");
                continue;
            }
            
            _aic.fi.name_max_len = 0;
            _aic.fi.uname_max_len = 0;
            _aic.fi.group_max_len = 0;
            _aic.fi.out_row = 0;

            regex_count = 0;
            while((rd=readdir(d))!=NULL) {
                stats_flag = 1;
                if (!_iargs.args[ARGS_LSALL]
                    && rd->d_name[0] == '.'
                ) {
                    continue;
                }
                strcpy(pathbuf, pcell->path);
                if (!pcell->is_root)
                    strcat(pathbuf, "/");

                if (pcell->plen + LINUX_NAME_LEN >= MAX_NAME_LEN)
                    continue;
                
                strcat(pathbuf, rd->d_name);

                if (lstat(pathbuf, &stmp)<0) {
                    perror("lstat");
                    continue;
                }
                if (_iargs.args[ARGS_REGEX]) {

                    if (regexec(_iargs.regcom,rd->d_name,1,_iargs.regmatch,0)!=0)
                    {
                        stats_flag = 0;
                        if (S_ISDIR(stmp.st_mode)) {
                            set_st_fbuf(&fbuf, &stmp, rd->d_name, pathbuf, cur_height+1);
                            set_fbuf_hide(&fbuf, 1);
                            add_to_flcache(&_aic.flcache, &fbuf);
                        } 
                    } else {
                        set_st_fbuf(&fbuf, &stmp, rd->d_name, pathbuf, cur_height+1);
                        add_to_flcache(&_aic.flcache, &fbuf);
                        regex_count += 1;
                    }

                } else {
                    set_st_fbuf(&fbuf, &stmp, rd->d_name, pathbuf, cur_height+1);
                    add_to_flcache(&_aic.flcache, &fbuf);
                }


                if ((_iargs.args[ARGS_STATIS]
                    || _iargs.args[ARGS_SHOWSTATS])
                    && stats_flag
                ) {
                    start_statis(&stmp, &_aic.stats);
                }

                caclt_fi(&fbuf, &_aic.fi, &stmp);

            }//end readdir
            closedir(d);
            out_flcache(&_aic.flcache, pcell->path, &_aic.fi);
            add_fbuf_dirs_to_plist(&_aic.flcache, &_pathlist);
            destroy_flcache(&_aic.flcache);
            if (!_iargs.args[ARGS_REGEX])
                printf("\n");
        }//end for
        
        pl = pl->next;
    }

  end_recur:;
    if (_iargs.args[ARGS_STATIS] || _iargs.args[ARGS_SHOWSTATS]) {
        out_statis(&_aic.stats);
    }

    return 0;
}

void help(void);

int main(int argc, char *argv[])
{

    init_infoargs(&_iargs);

    bzero(&_aic.stats, sizeof(_aic.stats));
    bzero(&_aic.fi, sizeof(struct format_info));
    init_flist_cache(&_aic.flcache);
    
    init_path_list(&_pathlist);

    struct winsize _wz;
    ioctl (1, TIOCGWINSZ, &_wz);
    if (_wz.ws_col > 0)
        _aic.fi.win_col = _wz.ws_col;
    if (_wz.ws_row > 0)
        _aic.fi.win_row = _wz.ws_row;

    char path_buffer[MAX_NAME_LEN] = {'\0'};

    struct stat stmp;
    struct file_buf fbuf;
    
    int recur_deep = 1;
    unsigned long len_buf = 0;

    for(int i=1;i<argc;i++) {
        if (strncmp(argv[i],"--deep=",7)==0) {
            recur_deep = atoi(argv[i]+7);
            if(recur_deep < 0)
                recur_deep=1;

            if (recur_deep > 1) {
                _iargs.args[ARGS_RECUR] = 1;
            }
        }
        else if (strcmp(argv[i],"--lnk")==0) {
            _iargs.args[ARGS_LINK] = 1;
            _iargs.args[ARGS_OUTMORE] = 1;
        }
        else if (strcmp(argv[i],"--path")==0) {
            _iargs.args[ARGS_PATH] = 1;
            _iargs.args[ARGS_OUTMORE] = 1;
        }
        else if (strcmp(argv[i], "--show-stats") == 0) {
            _iargs.args[ARGS_SHOWSTATS] = 1;
            _iargs.args[ARGS_STATIS] = 1;
        }
        else if (strcmp(argv[i],"--color")==0)
            _iargs.args[ARGS_COLOR] = 1;
        else if (strcmp(argv[i],"--sort")==0)
            _iargs.args[ARGS_SORT] = 1;
        else if(strcmp(argv[i],"--no-file")==0)
            _iargs.args[ARGS_REGNOFIL] = 1;
        else if(strcmp(argv[i],"--no-dir")==0)
            _iargs.args[ARGS_REGNODIR] = 1;
        else if (strncmp(argv[i],"--sort=",7)==0) {
            if (strlen(argv[i])==8) {
                if (argv[i][7] == SORT_BYTM
                    || argv[i][7] == SORT_BYCHTM
                    || argv[i][7] == SORT_BYSIZE
                    || argv[i][7] == SORT_BYNAME
                ){
                    _iargs.args[ARGS_SORT] = argv[i][7];
                } else {
                    _iargs.args[ARGS_SORT] = SORT_BYNAME;
                }
            } else {
                dprintf(2, "Error:unknow sort type\n");
                return 1;
            }
        }
        else if (strcmp(argv[i],"--regex")==0) {
            _iargs.args[ARGS_OUTMORE] = 1;
            i++;
            if (i >= argc) {
                dprintf(2,"Error:less argument -> --regex [REGEX]");
                return 2;
            }
            if (regcomp(_iargs.regcom, argv[i], REG_EXTENDED|REG_ICASE)!=0) {
                dprintf(2, "Error: compile regex -> %s\n",argv[i]);
                perror("regcomp");
                return 2;
            }
            _iargs.args[ARGS_REGEX] = 1;
            _iargs.args[ARGS_RECUR] = 1;
            recur_deep = 0;
        }
        else if (strcmp(argv[i], "-h")==0) {
            help();
            return 0;
        }
        else if (argv[i][0]=='-') {
            len_buf = strlen(argv[i]);
            for(int a=1; a<len_buf; a++){
                if (argv[i][a]=='a')
                    _iargs.args[ARGS_LSALL] = 1;
                else if (argv[i][a] == 'i') {
                    _iargs.args[ARGS_INO] = 1;
                    _iargs.args[ARGS_OUTMORE] = 1;
                }
                else if (argv[i][a] == 'm') {
                    _iargs.args[ARGS_MODE] = 1;
                    _iargs.args[ARGS_OUTMORE] = 1;
                }
                else if (argv[i][a] == 'p') {
                    _iargs.args[ARGS_PATH] = 1;
                    _iargs.args[ARGS_OUTMORE] = 1;
                }
                else if (argv[i][a] == 's') {
                    _iargs.args[ARGS_SIZE] = 1;
                    _iargs.args[ARGS_OUTMORE] = 1;
                }
                else if (argv[i][a] == 't')
                    _iargs.args[ARGS_STATIS] = 1;
                else if (argv[i][a] == 'f')
                    _iargs.args[ARGS_TYPE] = 1;
                else if (argv[i][a] == 'l') {
                    _iargs.args[ARGS_LONGINFO] = 1;
                    _iargs.args[ARGS_OUTMORE] = 1;
                }
                else if (argv[i][a] == 'r') {
                    _iargs.args[ARGS_RECUR] = 1;
                    if (recur_deep == 1)
                        recur_deep = 0;
                }
                else if (argv[i][a] == 'c') {
                    _iargs.args[ARGS_CREATM] = 1;
                    _iargs.args[ARGS_OUTMORE] = 1;
                }
                else { //不匹配则可能是目录/文件名称
                    
                }
            }
        }
        else if (access(argv[i], F_OK)==0) {
            if (lstat(argv[i],&stmp)<0)
                continue;

            len_buf = strlen(argv[i]);
            if (len_buf >= MAX_NAME_LEN) {
                dprintf(2,"Error: the length of path too long -> %s\n",argv[i]);
                continue;
            }
            if (S_ISDIR(stmp.st_mode)) {
                if (len_buf > 1 && argv[i][len_buf-1]=='/') {
                    strncpy(path_buffer, argv[i], len_buf-1);
                    path_buffer[len_buf-1] = '\0';
                } else {
                    strcpy(path_buffer, argv[i]);
                }
                add_path_list(&_pathlist, path_buffer, 1);
            } else {
                set_st_fbuf(&fbuf, &stmp, argv[i], NULL, 1);
                caclt_fi(&fbuf, &_aic.fi, &stmp);
                add_to_flcache(&_aic.flcache, &fbuf);
            }
        }
        else
            dprintf(2, "Error:unknow arguments -> %s\n", argv[i]);
    }

    if (_pathlist.end_ind == 0 && _aic.flcache.end_ind == 0)
        add_path_list(&_pathlist, ".", 1);
   
    if (_pathlist.end_ind == 1 
        && _aic.flcache.end_ind == 0 
        && _iargs.args[ARGS_RECUR]==0
    ) {
        _iargs.out_dir_flag = 0;
    }

    if (_aic.flcache.end_ind > 0) {
        out_flcache(&_aic.flcache, "", &_aic.fi);
    }

    recur_dir(recur_deep);

    destroy_path_list(_pathlist.next);
    destroy_flcache(&_aic.flcache);

    return 0;
}

#define HELP_INFO   "\
    --color : 支持颜色输出\n\
    --lnk : 如果是软链接输出目标文件\n\
    --sort : 排序，默认会开启\n\
    --deep : 递归深度，--deep=[NUMBER]\n\
    \n\
    -h : 帮助文档\n\
    -l : 长格式输出{mode hard-link user group filename size [target]}\n\
    -c : 创建时间\n\
    -a : 显示隐藏文件\n\
    -r : 递归显示目录\n\
    -s : 文件大小\n\
    -p : 显示路径\n\
"

void help() {
   printf("%s manual\n", PROGRAM_NAME);
   printf("%s\n", HELP_INFO);
   printf("  type flag:\n");
   list_type_info();
}
