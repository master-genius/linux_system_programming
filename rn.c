#include <stdio.h>
#include <stdlib.h>

int a();
int b();

int rnloop(int n) {
    printf("%d\n", n);
    return rnloop(n-1);
}

int a() {
    return b();
}

int b() {
    return a();
}


int main(int argc, char *argv[]) {

//    rnloop(1024);
        
    a();

	return 0;
}
