#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int match(char *regex, char *text);

int matchwhere(char *regegx, char *text);

int matchchar(char c, char *regex, char *text);


int match (char * regex, char *text) {
    if (regex[0] == '^') {
        return matchwhere(regex+1, text);
    }

    do {
        if (matchwhere(regex, text))
            return 1;
    }while (*text++ != '\0');

    return 0;
}

int matchwhere(char *regex, char *text) {
    if (regex[0] == '\0')
        return 1;

    if (regex[1] == '*')
        return matchchar(regex[0], regex+2, text);

    if (regex[0] == '$' && regex[1] == '\0')
        return *text == '\0';

    if (*text != '\0' &&(regex[0]=='.' || regex[0]==*text) )
        return matchwhere(regex+1, text);

    return 0;
}

int matchchar(char c, char *regex, char *text) {
    do {
        if (matchwhere(regex, text))
            return 1;
    } while(*text != '\0' && (*text++ == c || c == '.'));

    return 0;
}


int main(int argc, char *argv[]) {

    if (argc < 2) {
        dprintf(2, "Error: less regexp\n");
        return -1;
    }

    char buffer[2048] = {'\0'};

    while(1) {
        fgets(buffer, 2000, stdin);
        if (match(argv[1], buffer))
            printf("match : %s\n", buffer);
        else {
            printf("%s -- %s not match\n", argv[1], buffer);
        }
    }


	return 0;
}

