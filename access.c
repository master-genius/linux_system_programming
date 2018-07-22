#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{

    if (argc < 2) {
        dprintf(2, "Error: less arguments -> file/dir name\n");
        return 1;
    }

    for (int i=1; i < argc; i++) {
        if (access(argv[i], F_OK) < 0)
            dprintf(2, "Error: %s not exists\n", argv[i]);
        else
            printf("%s exists\n", argv[i]);
    }

    return 0;
}

