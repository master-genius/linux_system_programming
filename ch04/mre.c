#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>


#define REGEX_CHAR  ".^*$"

int match(char *, char *);
int matchreg(char *, char *);
int matchchar(char, char *, char *);

int match(char *regex, char *text) {

    if (regex[0] == '^')
        return matchreg(regex+1, text);
    else if(strchr(REGEX_CHAR, regex[0]) == NULL) {
        while(*text != '\0' && *text != regex[0])
            text++;
    }
    return matchreg(regex, text);
}

int matchreg(char *regex, char *text) {

    if (regex[0] == '\0')
        return 1;

    if (regex[0] == '$' && regex[1] == '\0')
        return *text == '\0';

    if (regex[1] == '*') {
        if (regex[0]=='.') {
            while(*text!='\0') {
                if (matchreg(regex+2, text))
                    return 1;
                text++;
            }
        } else {
            char c = regex[0];

            while ( *text!='\0' && *text==c ) {
                text++;
            }
        }
        return matchreg(regex+2, text);

    }

    if (regex[0] == '.' && *text!='\0') {
        text++;
        return matchreg(regex+1, text);
    }

    return matchchar(regex[0], regex, text);
}

int matchchar(char c, char *regex, char *text) {

    while(*text != '\0' && *text++ == c) {
        if (matchreg(regex+1, text))
            return 1;
        else
            return match(regex,text);
    }

    return 0;
}

#define STD_IN_KYBD     1
#define STD_IN_FIFO     2

#define BUF_LEN     4096

int main(int argc, char *argv[]) {

    if (argc < 2) {
        dprintf(2, "Error: less regexp\n");
        return -1;
    }

    int std_in_type = STD_IN_KYBD;
    struct stat st;
    if (fstat(0, &st) < 0) {
        perror("fstat");
        return -1;
    }

    if (S_ISFIFO(st.st_mode) )
        std_in_type = STD_IN_FIFO;



    char buffer[BUF_LEN] = {'\0'};
    int len;
    char *d;

    if (std_in_type == STD_IN_KYBD) {
        while(1) {
            fgets(buffer, BUF_LEN-1, stdin);
            len = strlen(buffer);
            if (buffer[len-1]=='\n')
                buffer[len-1] = '\0';

            if (match(argv[1], buffer)) {
                printf("match: %s\n", buffer);
            } else {
                printf("[%s] not match\n", buffer);
            }
        }
    } else if (std_in_type == STD_IN_FIFO) {
      read_match:;
        d = fgets(buffer, BUF_LEN-1, stdin);
        if (d == NULL)
            goto end_match;

        if (match(argv[1], buffer))
            printf("%s", buffer);

        goto read_match;
    }

end_match:;
	return 0;
}

