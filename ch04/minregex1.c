#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REGEX_CHAR  ".^*$"

int match(char *regex, char *text) {

    if (regex[0] == '\0')
        return 1;

    if (regex[0] == '^')
        return match(regex+1, text);

    if (regex[0] == '$' && regex[1] == '\0')
        return *text == '\0';

    if (regex[1] == '*') {
        if (regex[0]=='.') {
            while(*text!='\0') {
                if (match(regex+2, text))
                    return 1;
                text++;
            }
        }

        char c = regex[0];

        while ( *text!='\0' && *text==c ) {
            text++;
        }

        return match(regex+2, text);
    }

    if (regex[0] == '.' && *text!='\0')
        text++;

    int rlen = strlen(regex);
    int i=0;
    while(i<rlen && *text!='\0') {
        
        if (regex[i] == '\\') {
            i++;
            goto just_char;
        }

        if (strchr(REGEX_CHAR, regex[i]))
            return match(regex+i, text);
        
      just_char:;
        if (i<rlen && regex[i] != *text)
            return 0;

        i++;
        text++;
    }

    if (i == rlen)
        return 1;
    return match(regex+i, text);
}


int main(int argc, char *argv[]) {

    if (argc < 2) {
        dprintf(2, "Error: less regexp\n");
        return -1;
    }

    char buffer[2048] = {'\0'};
    int len;

    while(1) {
        fgets(buffer, 2000, stdin);
        len = strlen(buffer);
        if (buffer[len-1]=='\n')
            buffer[len-1] = '\0';

        if (match(argv[1], buffer)) {
            printf("match: %s\n", buffer);
        } else {
            printf("[%s] not match\n", buffer);
        }
    }

	return 0;
}

