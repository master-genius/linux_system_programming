#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TYPE_DIR        '/'
#define TYPE_LNK        '@'
#define TYPE_FIFO       '|'
#define TYPE_SOCK       '='
#define TYPE_EXEC       '*'
#define TYPE_CHR        '%'
#define TYPE_BLK        '#'

#define TYPE_INFO   "\
    / : dir\n\
    @ : symble link\n\
    | : FIFO(PIPE)\n\
    = : socket\n\
    * : exec\n\
    %% : character device\n\
    # : block device\n"

void list_type_info() {
    printf(TYPE_INFO);
}

int main(void) {
    list_type_info();
    return 0;
}

