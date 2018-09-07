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


#define PROGRAM_NAME    "look"

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
#define STDOUT_FIPI     2

#define MAX_NAME_LEN    2048
#define PATH_CELL_END   16

#define SWAP(a,b)   tmp=a;a=b;b=tmp;

void qsort_core(void * base, int start, int end,
    unsigned int size, int (*comp)(const void *, const void *)
);

void vqsort(void* base, unsigned int nmemb, unsigned int size, 
    int(*comp)( const void *, const void *)
) {
    qsort_core(base, 0, nmemb/size - 1, size, comp);
}

//paradigm quick sort
void qsort_core(void * base, int start, int end,
    unsigned int size, int (*comp)(const void *, const void *)
) {
    if (start >= end) {
        return ;
    }
    
    int med = (end+start)/2;
    int k = start;
    int j;
    char tmp;
    char * b = base;

    for (int i=0;i<size;i++) {
        SWAP(b[med*size+i],b[start*size+i]);
    }

    for(j=start+1;j<=end;j++) {
        if (comp(b+j*size,b+start*size) < 0) {
            k += 1;
            if (k==j)continue;
            for(int i=0;i<size;i++) {
                SWAP(b[k*size+i],b[j*size+i]);
            }
        }
    }

    for (int i=0; i<size; i++) {
        SWAP(b[k*size+i],b[start*size+i]);
    }

    qsort_core(base, start, k-1, size, comp);
    qsort_core(base, k+1, end, size, comp);
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

    if (plast->end_ind < PATH_CELL_END) {
        pcell = plast->pce+plast->end_ind;
        plast->end_ind + 1;
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
};

int set_stdout_type(int * stdo) {
    struct stat dst;
    if (fstat(1, &dst) < 0) {
        perror("fstat");
        return -1;
    }

    if (S_ISFIFO(dst.st_mode))
        *stdo = STDOUT_FIPI;
    else
        *stdo = STDOUT_SCRN;

    return 0;
}

void init_infoargs(struct infoargs * ia) {
    bzero(ia->args, sizeof(char)*ARGS_END);
    ia->stdout_type = STDOUT_SCRN;
    set_stdout_type(&ia->stdout_type);
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
    char uname[40];
    char group[40];
    int height;
    struct stat st;
};

#define BUFF_LEN    4096

struct file_buf_list {
    struct file_buf fbuf;
    int use_status;
    struct file_buf_list * next;
};

struct file_list_cache {
    int end_ind;
    struct file_buf fcache[BUFF_LEN+1];
    int list_count;
    struct file_buf_list fbhead;
    struct file_buf_list * flast;

    //for sort
    struct file_buf *fba;
};

struct format_info {
    int count;
    int name_max_len;
    int uname_max_len;
    int group_max_len;
    int win_row;
    int win_col;
    int out_row;
};


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
    }
    if (path)
        strcpy(fbuf->path, path);

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
    
    return 0;
}

int add_to_flcache(struct file_list_cache * flcache, struct file_buf * fbuf) {
    struct file_buf * fbtmp = NULL;
    struct file_buf_list *fl = NULL;
    if (flcache->end_ind < BUFF_LEN) {
        fbtmp = &flcache->fcache[flcache->end_ind];
        flcache->end_ind += 1;

    } else {
        fl = (struct file_buf_list *)malloc(sizeof(struct file_buf_list));
        if (fl == NULL) {
            perror("malloc");
            return -1;
        }
        fl->next = NULL;
        flcache->flast = fl;
        flcache->list_count += 1;
        fbtmp = &fl->fbuf;
    }
    memcpy(fbtmp,fbuf, sizeof(struct file_buf));
    return 0;
}

void clear_flcache(struct file_list_cache *flcache) {
    flcache->end_ind = 0;
    flcache->flast = &flcache->fbhead;
}

void destroy_flcache(struct file_list_cache *flcache) {
    flcache->end_ind = 0;
    struct file_buf_list * fbtmp = flcache->fbhead.next;
    struct file_buf_list * fbtmp2;
    while(fbtmp) {
        if (fbtmp->next)
            fbtmp2 = fbtmp->next;
        free(fbtmp);
        fbtmp = fbtmp2;
    }
    flcache->fbhead.next = NULL;
    flcache->flast = &flcache->fbhead;
    free(flcache->fba);
    flcache->fba = NULL;
}

int fbuf_name_comp(const void *a, const void *b) {
    struct file_buf const *fa = a;
    struct file_buf const *fb = b;
    return strcmp(fa->path, fb->path);
}

int fbuf_size_comp(const void *a, const void *b) {
    struct file_buf const *fa = a;
    struct file_buf const *fb = b;

    return ((fa->st.st_size == fb->st.st_size)
            ? 0 : (
                (fa->st.st_size > fb->st.st_size)? 1 : -1
            )
        );
}

int fbuf_sort(struct file_list_cache * flcache, int sort_flag) {
    int total_count = flcache->end_ind + flcache->list_count;
    flcache->fba = (struct file_buf*)malloc(sizeof(struct file_buf)*total_count);
    if (flcache->fba==NULL) {
        return -1;
    }

    int (*comp)(const void *, const void *);
    if (sort_flag == SORT_BYSIZE) {
        comp = fbuf_size_comp;
    } else {
        comp = fbuf_name_comp;
    }
    
    vqsort(flcache->fba, sizeof(struct file_buf)*total_count, 
            sizeof(struct file_buf), comp);

    return 0;
}


int
add_fbuf_dirs_to_plist(struct file_list_cache* flcache, 
        struct path_list* plist) 
{
    
    struct file_buf *fbtmp = NULL;
    struct file_buf_list * fl;

    int i=0;

    for (i=0; i<flcache->end_ind; i++) {
        fbtmp = &flcache->fcache[i];
        if (S_ISDIR(fbtmp->st.st_mode)) {
            add_path_list(plist, fbtmp->path, fbtmp->height);
        }
    }
    
    if (flcache->list_count > 0) {
        fl = flcache->fbhead.next;
        while(fl) {
            fbtmp = &fl->fbuf;
            if (S_ISDIR(fbtmp->st.st_mode)) {
                add_path_list(plist, fbtmp->path, fbtmp->height);
            }
        }
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
        printf("\e[2;36m%s",pname);
    else if (S_ISREG(fb->st.st_mode) && access(fb->path,X_OK)==0)
        printf("\e[1;35m%s",pname);
    else
        printf("%s",pname);
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

    if (_iargs.args[ARGS_INO]) {
        count = sprintf(outline + posi, "%-9lu ", fbuf->st.st_ino);
        posi += count;
    }
    
    if (_iargs.args[ARGS_MODE] || _iargs.args[ARGS_LONGINFO]) {
        count = sprintf(outline + posi, "%o ", fbuf->st.st_mode & 0777);
        posi += count;
    }

    if (_iargs.args[ARGS_LONGINFO]) {
        count = sprintf(outline + posi, "%-2lu ", fbuf->st.st_nlink);
        posi += count;
    }
    
    if (_iargs.args[ARGS_USRGRP] || _iargs.args[ARGS_LONGINFO]) {
        sprintf(fmt_str, "%%-%ds %%-%ds", fi->uname_max_len, fi->group_max_len);
        count = sprintf(outline + posi, fmt_str, fbuf->uname, fbuf->group);
        posi += count;
    }

    if (_iargs.args[ARGS_CREATM]) {
        time_t t = time(NULL);
        struct tm *ct = localtime(&t);
        count = sprintf(outline+posi, "%d.%2d.%2d %2d:%2d ", ct->tm_year+1900, 
                ct->tm_mon+1, ct->tm_mday, ct->tm_hour, ct->tm_min);
        posi += count;
    }

    
    int fmt_name_len = 0;
    if ((_iargs.args[ARGS_PATH] || _iargs.args[ARGS_REGEX]) 
        && ppath
    ) {

        count = sprintf(outline + posi, "%s/", ppath);
        posi += count;
    }

    fmt_name_len = strlen(fbuf->name) + fi->name_max_len;
    sprintf(fmt_str, "%%-%ds ", fmt_name_len);
    count = sprintf(outline+posi, fmt_str, fbuf->name);
    posi += count;

    char flag = '\0';
    if (_iargs.args[ARGS_TYPE]) {
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
        if (flag > 0) {
            count = sprintf(outline+posi, "%c",flag);
            posi += count;
        }
    }

    if (_iargs.args[ARGS_SIZE] || _iargs.args[ARGS_LONGINFO]) {
        format_size(fbuf->st.st_size, outline+posi);
        posi = strlen(outline);
    }

    if (_iargs.args[ARGS_LINK] || _iargs.args[ARGS_LONGINFO]) {
        
        char _path_buffer[MAX_NAME_LEN];
        int link_len = readlink(fbuf->path, _path_buffer, MAX_NAME_LEN-1);
        if (link_len > 0) {
            _path_buffer[link_len] = '\0';
            count = sprintf(outline+posi, "-> %s", _path_buffer);
            posi += count;
        }
    }

}

int out_flcache(struct file_list_cache *flcache, 
        char *path, struct format_info *fi)
{

    char outline[MAX_OUTLINE_LEN+128];
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

    int max_len;
    max_len = fi->name_max_len + 2;
    fi->out_row = fi->win_col/max_len;

    char fmt_str[128] = {'\0'};
    int next_line = 0;
    int total = flcache->end_ind + flcache->list_count;
    
    for(int i=0; i<total; i++) {
        out_info(&flcache->fba[i], path, fi, outline);
        if (_iargs.args[ARGS_OUTMORE]) {
            printf("%s\n", outline);
        } else {
            sprintf(fmt_str, "%%-%ds ", fi->name_max_len+1);

            printf(fmt_str, outline);
            next_line += 1;
            if (next_line >= fi->out_row) {
                next_line = 0;
                printf("\n");
            }
        }
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


void help(void);

void help() {

}

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
    int len_buf = 0;
    while (pl) {
 
        for(i=0; i<pl->end_ind; i++) {
            if (deep > 0 && cur_height > deep)goto end_recur;
            cur_height = pcell->height + 1;
            pcell = &pl->pce[i];
            
            d = opendir(pcell->path);
            if (!d) {
                perror("opendir");
                continue;
            }
            
            bzero(&_aic.fi, sizeof(struct format_info));

            while((rd=readdir(d))!=NULL) {
                if (!_iargs.args[ARGS_LSALL]
                    && rd->d_name[0] == '.'
                ) {
                    continue;
                }
                strcpy(pathbuf, pcell->path);
                if (!pcell->is_root)
                    strcat(pathbuf, "/");
                
                strcat(pathbuf, rd->d_name);

                if (lstat(pathbuf, &stmp)<0) {
                    perror("lstat");
                    continue;
                }

                if (_iargs.args[ARGS_STATIS]
                    || _iargs.args[ARGS_SHOWSTATS]
                ) {
                    start_statis(&stmp, &_aic.stats);
                }
                
                set_st_fbuf(&fbuf, &stmp, rd->d_name, pathbuf, cur_height);
                add_to_flcache(&_aic.flcache, &fbuf);

                len_buf = strlen(rd->d_name);
                if (_aic.fi.name_max_len < len_buf)
                    _aic.fi.name_max_len = len_buf;

                len_buf = strlen(fbuf.uname);
                if (_aic.fi.uname_max_len < len_buf)
                    _aic.fi.uname_max_len = len_buf;

                len_buf = strlen(fbuf.group);
                if (_aic.fi.group_max_len < len_buf)
                    _aic.fi.group_max_len = len_buf;

            }//end readdir

            out_flcache(&_aic.flcache, pcell->path, &_aic.fi);
            destroy_flcache(&_aic.flcache);
        }//end for

        pl = pl->next;
    }

  end_recur:;

    return 0;
}

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
    int len_buf = 0;

    for(int i=1;i<argc;i++) {
        if (strncmp(argv[i],"--deep=",7)==0) {
            recur_deep = atoi(argv[i]+7);
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
                else if (argv[i][a] == 'i')
                    _iargs.args[ARGS_INO] = 1;
                else if (argv[i][a] == 'm')
                    _iargs.args[ARGS_MODE] = 1;
                else if (argv[i][a] == 'p')
                    _iargs.args[ARGS_PATH] = 1;
                else if (argv[i][a] == 's')
                    _iargs.args[ARGS_SIZE] = 1;
                else if (argv[i][a] == 't')
                    _iargs.args[ARGS_STATIS] = 1;
                else if (argv[i][a] == 'f')
                    _iargs.args[ARGS_TYPE] = 1;
                else if (argv[i][a] == 'l')
                    _iargs.args[ARGS_LONGINFO] = 1;
                else if (argv[i][a] == 'r') {
                    _iargs.args[ARGS_RECUR] = 1;
                    recur_deep = 0;
                }
                else if (argv[i][a] == 'c')
                    _iargs.args[ARGS_CREATM] = 1;
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
                }

                add_path_list(&_pathlist, path_buffer, 1);
            } else {
                set_st_fbuf(&fbuf, &stmp, argv[i], NULL, 1);
               
                if (_aic.fi.name_max_len < len_buf)
                    _aic.fi.name_max_len = len_buf;
                
                len_buf = strlen(fbuf.uname);
                if (_aic.fi.uname_max_len < len_buf)
                    _aic.fi.uname_max_len = len_buf;

                len_buf = strlen(fbuf.group);
                if (_aic.fi.group_max_len < len_buf)
                    _aic.fi.group_max_len = len_buf;

                add_to_flcache(&_aic.flcache, &fbuf);
            }
        }
        else
            dprintf(2, "Error:unknow arguments -> %s\n", argv[i]);
    }


    printf("ok\n");
    if (_pathlist.end_ind == 0 && _aic.flcache.end_ind == 0)
        add_path_list(&_pathlist, ".", 1);
    
    if (_aic.flcache.end_ind > 0) {
        out_flcache(&_aic.flcache, "", &_aic.fi);
    }

    recur_dir(recur_deep);
   

    destroy_path_list(_pathlist.next);
    destroy_flcache(&_aic.flcache);

    return 0;
}

/*
*/

