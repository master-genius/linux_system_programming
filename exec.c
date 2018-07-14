#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>


int main(int argc, char * argv[])
{
    
    char * path = getenv("PATH");
    printf("%s\n",path);


    return 0;
}

