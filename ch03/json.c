#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>



struct json {
    int jtype;
    char *lname;
    char name[256];
    int name_flag;
    enum {
        int num;
        double dnum;
        long long dnum;
        struct json *jn;
    }val;
    struct json * next;
};







int main(int argc, char *argv[]) {

    return 0;
}

