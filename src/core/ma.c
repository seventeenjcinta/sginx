#include <stdio.h>
// #include "sgx_palloc.h"

int main()
{
    int ma[6] = {1, 2, 3, 4, 5, 6};
    int *p;

    p = ma;
    *p = 3;
    p ++;
    for(int i = 0; i < 6; i ++) {
        printf("%d\n", ma[i]);
    }
    printf("%d\n", *p);

    return 0;
}