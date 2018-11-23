#include <stdio.h>
#include <stdlib.h>

int nsum(int n) {
    if (n > 0) {
        return n + nsum(n-1);
    }
    return 0;
}

int fibo(int n) {
    if (n < 3) {
        return 1;
    } else {
        return fibo(n-1) + fibo(n-2);
    }
}


int main(int argc, char *argv[]) {
    printf("sum : %d\n", nsum(5));

    printf("fibo(5) : %d  , fibo(8) : %d\n", fibo(5), fibo(8));

	return 0;
}
