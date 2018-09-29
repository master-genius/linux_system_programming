#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int strrotstr(char *s1, char *s2) {
    char *buf = (char*)malloc(sizeof(char)*strlen(s1)*2 + 1);
    if (buf == NULL) {
        return -1;
    }

    int is_in = 0;

    strcpy(buf, s1);
    strcat(buf, s1);

    if (strstr(buf, s2))
        is_in = 1;

    free(buf);

    return is_in;
}


int main(int argc, char *argv[]) {

    if (argc < 3) {
        dprintf(2, "Error: string at least 2\n");
        return -1;
    }

    printf("%d\n", strrotstr(argv[1], argv[2]));

	return 0;
}

