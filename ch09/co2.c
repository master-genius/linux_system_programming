#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>

#define HELP_INFO   "\
dup : copy file or dir to another path\n\
    unlike command cp , you don't need -R to copy a dir.\n\
    usage : dup  [FILE_NAME or DIR_NAME] [TARGET_PATH]\n\
    example :\n\n\
    dup  a  b\n\
        copy a to b\n\
        \n\
    dup  a  x/b\n\
        copy a to dir x and rename a to b\n\
        \n\
    dup  a  b  c/  x/ \n\
        copy file a b and dir c to dir x/,if x not exists, x will be created.\n\
        \n\
    dup  c/  x\n\
        if x exists, c/ will into x, if x not exists, x will be created,\n\
        then, copy all files from c/ to x/.\n\
"

void help(void) {
    printf("%s\n", HELP_INFO);
}

#define CO_BUFFER_LEN      8192

#define MAX_NAME_LEN        2048

/*
    设计方式：
        master进程只负责创建目录，并把文件路径和目标路径放进消息队列。
        两个子进程负责监听消息队列，从队列里获取数据并进行复制。
*/

int copy_file_core(char *src, char *dest);

int copy_file(char *src, char *dest);

int copy_dir(char *src, char *dest);

int copy_control(char *src, char *dest);


int copy_file_core(char *src, char *dest) {
    
    int errcode = 0;

    struct stat st;
    if (lstat(src, &st) < 0) {
        perror("lstat");
        return -1;
    }

    char data[CO_BUFFER_LEN+1] = {'\0'};
    int count = 0;

    if (S_ISLNK(st.st_mode)) {
        count = readlink(src, data, MAX_NAME_LEN);
        if (count < 0) {
            dprintf(2, "Error: read link failed -> %s\n", src);
            return -1;
        }

        if (symlink(data, dest) < 0) {
            perror("symlink");
            return -1;
        }
    } else if (S_ISREG(st.st_mode)) {
        int fd = 0;
        if (access(dest, F_OK) == 0) {
            chmod(dest, 0777 & st.st_mode);
        }
        fd = open(dest, O_CREAT|O_RDWR|O_TRUNC, 0777 & st.st_mode);
        if (fd < 0) {
            perror("open");
            return -1;
        }

        int fo = open(src, O_RDONLY);
        if (fo < 0) {
            perror("fo");
            errcode = -1;
            goto end_copy;
        }

        while((count=read(fo, data, CO_BUFFER_LEN)) > 0) {
            if (write(fd, data, count) < 0) {
                perror("write");
                errcode = -1;
                break;
            }
        }
      end_copy:;
        close(fo);
        //fsync(fd);
        close(fd);
    } else if (S_ISFIFO(st.st_mode)) {
        if (mknod(dest, S_IFIFO|(0777 & st.st_mode), 0) < 0) {
            perror("mknod");
            return -1;
        }
    } else if (S_ISSOCK(st.st_mode)) {
        if (mknod(dest, S_IFSOCK|(0777&st.st_mode), 0) < 0) {
            perror("mknod");
            return -1;
        }
    }

    return errcode;
}

int copy_file(char *src, char *dest) {
    if (access(dest, F_OK)==0) {
        struct stat st;
        if (stat(dest, &st)<0) {
            perror("stat");
            return -1;
        }
        if (S_ISDIR(st.st_mode)) {
            char destname[MAX_NAME_LEN] = {'\0'};
            strcpy(destname, dest);
            int slen = strlen(src);
            int i = slen-1;
            while(i > 0 && src[i]!='/')i--;
            if(src[i]=='/')i++;

            strcat(destname, "/");
            if ((slen - i + strlen(dest)) >= MAX_NAME_LEN)
                return -1;
            strcat(destname, src+i);
            return copy_file_core(src, destname);
        }

    }
    return copy_file_core(src, dest);
}

int copy_dir(char *src, char *dest) {

    char name_buf[MAX_NAME_LEN];
    char dest_buf[MAX_NAME_LEN];
    
    struct stat st;

    DIR *d = NULL;
    struct dirent *rd = NULL;
    d = opendir(src);
    if (d==NULL) {
        perror("opendir");
        return -1;
    }

    while((rd=readdir(d))!=NULL) {

        strcpy(name_buf, src);
        strcat(name_buf, "/");

        if (strlen(rd->d_name) + strlen(name_buf) >= MAX_NAME_LEN)
            continue;

        strcat(name_buf, rd->d_name);

        if (stat(name_buf, &st)) {
            dprintf(2, "%s ", name_buf);
            perror("stat");
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            strcpy(dest_buf, dest);
            strcat(dest_buf, "/");
            strcat(dest_buf, rd->d_name);
            if (mkdir(dest_buf, st.st_mode & 0777) < 0)
                continue;
            copy_dir(name_buf, dest_buf);
        } else {
            copy_file(name_buf, dest);
        }

    }

    closedir(d);

    return 0;
}

int copy_control(char *src, char *dest) {
    if (access(src, F_OK) < 0) {
        return -1;
    }

    int slen = strlen(src);
    int dlen = strlen(dest);
    if (slen > 1 && src[slen-1] == '/') {
        src[slen-1] = '\0';
        slen -= 1;
    }

    if (dlen > 1 && dest[dlen-1] == '/') {
        dest[dlen-1] = '\0';
        dlen -= 1;
    }

    struct stat st;
    
    if (stat(src, &st) < 0) {
        dprintf(2, "%s ", src);
        perror("stat");
        return -1;
    }

    if (S_ISDIR(st.st_mode)) {
        char *real_dest = dest;
        char dest_buf[MAX_NAME_LEN];

        if (access(dest, F_OK) < 0) {
            if (mkdir(dest, 0777 & st.st_mode) < 0)
                return -1;
        } else {
            if (stat(dest, &st) < 0) {
                dprintf(2, "%s ", dest);
                perror("stat");
                return -1;
            }

            if (!S_ISDIR(st.st_mode)) {
                dprintf(2, "%s is not a directory\n", dest);
                return -1;
            }
            strcpy(dest_buf, dest);
            strcat(dest_buf, "/");
            int i = slen-1;
            while(i > 0 && src[i]!='/')i--;
            if (src[i]=='/')
                i++;
            if ((dlen + 1 + slen - i) >= MAX_NAME_LEN) {
                return -1;
            }
            strcat(dest_buf, src+i);
            if (lstat(src, &st) < 0) {
                return -1;
            }
            if (mkdir(dest_buf, st.st_mode & 0777) < 0) {
                dprintf(2, "%s ", dest_buf);
                perror("mkdir");
                return -1;
            }
            real_dest = dest_buf;
        }
        return copy_dir(src, real_dest);
    } else {
        return copy_file(src, dest);
    }
}


int main(int argc, char *argv[]) {

    if (argc < 3) {
        help();
        return 0;
    }

    char *dest = argv[argc-1];
    struct stat st;
    if (access(dest, F_OK)==0) {
        if (stat(dest, &st) < 0) {
            dprintf(2, "%s ", dest);
            perror("stat");
            return -1;
        }
        
        if (!S_ISDIR(st.st_mode) && argc > 3) {
            dprintf(2, "Error: dest not a dir\n");
            return -1;
        }
    } else {
        if (argc > 3) {
            dprintf(2, "Error: %s not exists\n", dest);
            return -1;
        }
    }

    for (int i=1; i<argc-1; i++) {
        if (access(argv[i], F_OK) < 0) {
            dprintf(2, "Error:%s not exists\n", argv[i]);
            continue;
        }
        if (copy_control(argv[i], dest) < 0) {
            return -1;
        }
    }

	return 0;
}

