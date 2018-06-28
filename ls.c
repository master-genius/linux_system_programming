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
#define TYPE_FIFO       '='
#define TYPE_SOCK       '&'
#define TYPE_EXEC       '*'

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

#define ARGS_END        32

#define MAX_NAME_LEN    2049

char _args[ARGS_END] = {0};

struct path_list {
    char path[MAX_NAME_LEN];
    int height;
    struct path_list * next;
};

//this struct is cache the file for sort
struct file_list {
    char name[MAX_NAME_LEN];
    unsigned int count;
    struct file_list * next;
};
static int _file_count = 0;

struct file_buf {
    char name[MAX_NAME_LEN];
    struct stat st;
};

#define BUFF_LEN    256
//file list buffer
struct file_buf _files_buf[BUFF_LEN];

struct statis {
    unsigned long long dir_count;
    unsigned long long file_count;
    unsigned long long fifo_count;
    unsigned long long link_count;
    unsigned long long sock_count;
    unsigned long long total_count;
    unsigned long long total_size;
};

int init_path_list(char * path, struct path_list* pl);

int recur_dir(struct path_list* pl, int max_height, struct statis* stats);

void out_info(char* name, char* path, struct stat* st);

void out_statis(struct statis* stats);

void out_control(char * name, char* path, struct stat * st);

void destroy_path_list(struct path_list* pl);

void destroy_file_list(struct file_list * fl);

void help(void)
{
    char *help_info[] = {
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
    struct path_list pl;
    pl.next = NULL;

    struct statis stats;

    bzero(&stats, sizeof(stats));

    int recur_deep = 1;
    for(int i=1;i<argc;i++) {
        if (strcmp(argv[i],"-s")==0)
            _args[ARGS_SIZE] = 1;
        else if (strcmp(argv[i],"-r")==0)
            _args[ARGS_RECUR] = 1;
        else if (strcmp(argv[i],"-a")==0)
            _args[ARGS_LSALL] = 1;
        else if (strncmp(argv[i],"--deep=",7)==0) {
            recur_deep = atoi(argv[i]+7);
            recur_deep = (recur_deep<=0)?1:recur_deep;
        }
        else if (strcmp(argv[i],"--lnk")==0)
            _args[ARGS_LINK] = 1;
        else if (strcmp(argv[i],"--path")==0)
            _args[ARGS_PATH] = 1;
        else if (strcmp(argv[i],"-t")==0)
            _args[ARGS_STATIS] = 1;
        else if (strcmp(argv[i],"-m")==0)
            _args[ARGS_MODE] = 1;
        else if (strcmp(argv[i], "-h")==0) {
            help();
            return 0;
        }
        else if (access(argv[i], F_OK)==0)
            init_path_list(argv[i], &pl);
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
    struct path_list * ptmp = NULL;
    ptmp = (struct path_list*)malloc(sizeof(struct path_list));
    if (ptmp == NULL) {
        perror("malloc");
        return -1;
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
    
    while (end_path!=NULL && end_path->next!=NULL) {
        end_path = end_path->next;
    }

    DIR* d = NULL;
    struct dirent * rd = NULL;
    struct stat sttmp;
    char nbuf[MAX_NAME_LEN] = {'\0'};

    while (cur!=NULL) {
        if(max_height>0 && cur->height >= max_height)break;
        
        d = opendir(cur->path);
        if (d==NULL) {
            perror("opendir");
            continue;
        }
        while((rd = readdir(d))!=NULL) {
            strcpy(nbuf, cur->path);
            strcat(nbuf, "/");
            if (strlen(rd->d_name)+strlen(nbuf)>MAX_NAME_LEN) {
                break;
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
            
            stats->total_count++;
            stats->total_size += sttmp.st_size;

            if (S_ISDIR(sttmp.st_mode)) {
                path_tmp = (struct path_list*)malloc(sizeof(struct path_list));
                if (path_tmp==NULL)
                    return -1;
                path_tmp->next = NULL;

                path_tmp->height = cur->height+1;
                strcpy(path_tmp->path, cur->path);
                strcat(path_tmp->path, "/");
                strcat(path_tmp->path, rd->d_name);
                
                end_path->next = path_tmp;
                end_path = path_tmp;
                path_tmp = NULL;
            } else {
                if (S_ISREG(sttmp.st_mode))
                    stats->file_count++;
                else if (S_ISLNK(sttmp.st_mode))
                    stats->link_count++;
                else if (S_ISFIFO(sttmp.st_mode))
                    stats->fifo_count++;
                else if (S_ISSOCK(sttmp.st_mode))
                    stats->sock_count++;

            }
            //printf("%s\n", rd->d_name);
            //out_info(rd->d_name, cur->path, &sttmp);
            out_control(rd->d_name, cur->path, &sttmp);
        }
        closedir(d);
        cur = cur->next;
    } 
}

void out_control(char * name, char* path, struct stat * st) {
    if(_args[ARGS_SORT]) {

    } else {
        out_info(name, path, st);
    }
}

void out_info(char * name, char * path, struct stat * st) {
    if (_args[ARGS_PATH])
        printf("%s/",path);
    printf("%s", name);
    if (_args[ARGS_SIZE]) {
        if (st->st_size <= 1024)
            printf(" %dB",st->st_size);
        else if (st->st_size > 1024 && st->st_size < 1048576)
            printf(" %.2lfK", (double)st->st_size/1024);
        else
            printf(" %.2lfM",(double)st->st_size/1048576);
    }
    if(_args[ARGS_MODE])
        printf(" %o", st->st_mode);
    printf("\n");
}

void out_statis(struct statis * stats) {
    printf("-- statis --\n");
    char * count_name[] = {
        "regular file",
        "fifo file",
        "link file",
        "sock file",
        "directory",
        "\0"
    };

    unsigned long long count_ind[] = {
        stats->file_count, stats->fifo_count, stats->link_count, 
        stats->sock_count, stats->dir_count
    };
    int i=0;
    while (strcmp(count_name[i],"\0")!=0) {
        if (count_ind[i]>0)
            printf("%s : %ld\n", count_name[i], count_ind[i]);
        i++;
    }

    printf("total count : %ld\n", stats->total_count);
    printf("total size : ");
    if (stats->total_size <= 1024)
        printf(" %dB\n",stats->total_size);
    else if (stats->total_size > 1024 && stats->total_size < 1048576)
        printf(" %.2lfK\n", (double)stats->total_size/1024);
    else
        printf(" %.2lfM\n",(double)stats->total_size/1048576);

}

void destroy_file_list(struct file_list * fl) {

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
