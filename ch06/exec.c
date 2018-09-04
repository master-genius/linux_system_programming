#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char * argv[])
{

    char * cmd_path = "/bin/uname";
    char * cmd_argv[] = {"/bin/uname", "-a", NULL};
    
    if (execv(cmd_path, cmd_argv)<0) {
        perror("execv");
        return 1;
    }

    return 0;
}

