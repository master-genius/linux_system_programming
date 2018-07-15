#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

char *_path[] = {
    "/bin",
    "/sbin",
    "/usr/bin",
    "/usr/sbin",
    "/usr/local/bin",
    "/usr/local/sbin",
    "\0"
};

int main(int argc, char * argv[])
{
    char * path = getenv("HOME");


    return 0;
}

