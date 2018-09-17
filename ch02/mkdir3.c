#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void help(void)
{
    char *help_info[] = {
        "创建目录，支持参数--help，--mode\n",
        "--help：输出帮助信息\n",
        "--mode=[MODE]：设定创建目录的权限，MODE应该是一个三位的数字，",
        "否则程序会报错，数字是0-7的范围。比如，754表示rwxr-xr--。\n",
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

    for(i=1;i<argc;i++) {
        if (strcmp(argv[i],"--help")==0) {
            help();
            return 0;
        } else if (strncmp(argv[i],"--mode=",7)==0) {
            if (mode_flag > 0) {
                dprintf(2,"Error: too many --mode\n");
                return 1;
            }

            if (strlen(argv[i]+7)!=3) {
                dprintf(2, "Error: mode is wrong\n");
                return -1;
            }
            for (int k=0;k<3;k++) {
                tmp = argv[i][7+k];
                if (tmp < '0' || tmp > '7') {
                    dprintf(2, "Error: mode number must in [0,7]\n");
                    return -1;
                }
                mode_buf += (tmp-48)*(1<<(3*(2-k)));
            }

            mode_flag = i;
        }
    }

    if (mode_flag>0 && argc==2) {
        dprintf(2,"Error: less DIR_NAME\n");
        return -1;
    }

    if (mode_flag>0 && mode_buf > 0)
        mode = mode_buf;

    for (i=1;i<argc;i++) {
        
        if (i==mode_flag)continue;

        if (mkdir(argv[i],mode) < 0) {
            perror("mkdir");
            return -1;
        }
    }

    return 0;
}

