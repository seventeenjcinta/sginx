#include <unistd.h>
#include <stdio.h>
#include "sgx_thread_pool.h"

void *f(void *arg)
{
    printf("thread %d\n", (int)arg);
}

int main()
{
    if(sgx_thread_pool_create(5)) {
        printf("thread_pool create faild.\n");
        exit(1);
    }
    for(int i = 0; i < 10; i ++) {
        sgx_thread_pool_add_work(f, (void *)i);
    }
    sleep(2);
    sgx_thread_destroy();

    return 0;
}