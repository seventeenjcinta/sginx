#include <stdio.h>
#include "sgx_priority_queue.c"


/// https://www.luogu.org/problem/P3378
/// 洛谷给了你多少钱，我 bzoj 给双倍    

int main()
{
    int n;
    
    while(scanf("%d", &n) == 1) {
        int ops;
        sgx_priority_queue Q;

        sgx_priority_queue_init(&Q, sgx_priority_queue_greater, SGX_PRIORITY_QUEUE_DEFAULT_SIZE);
        for(int i = 0; i < n; i ++) {
            scanf("%d", &ops);
            if(ops == 2) {
                void *ans;

                ans = sgx_priority_queue_top(&Q);
                if(ans == NULL) {
                    printf("0\n");
                }
                else {
                    printf("%d\n", -*(int *)ans);
                }
            }
            else if(ops == 1) {
                int *value;
                int x;

                value = (int *)malloc(sizeof(int));
                scanf("%d", &x);
                *value = -x;
                sgx_priority_queue_push(&Q, value);
            }
            else if(ops == 3) {
                sgx_priority_queue_pop(&Q);
            }
            // priority_queue_show(&Q);
        }
        sgx_priority_queue_clear(&Q);
    }

    return 0;
}