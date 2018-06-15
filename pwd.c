#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/limits.h>

int main (int argc, char *argv[])
{
    char buf[PATH_MAX] = {'\0'};
    if (getcwd(buf,PATH_MAX)==NULL) {
        perror("getcwd");
        return -1;
    }

    printf("%s\n",buf);

    return 0;
}
