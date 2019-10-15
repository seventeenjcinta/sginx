#include <stdlib.h>
#include "sgx_core.h"
#include "sgx_priority_queue.h"

static void priority_queue_show(sgx_priority_queue *pqueue)
{
    printf("the max_size is %d\n", pqueue -> max_size);
    printf("the size is %d\n", pqueue -> size);
    for(size_t i = 1; i <= pqueue -> size; i ++) {
        printf("node[%d] = %d\n", i, **(int **)(pqueue -> node + i));
    }
}

/// 下标从 1 开始
static int priority_queue_resize(sgx_priority_queue *pqueue, size_t new_size)
{
    void **new_pqueue;
    
    if(new_size <= pqueue -> size) {
        sgx_log_err("resize: new_size to samll");

        return -1;
    }
    new_pqueue = (void **)malloc(sizeof(void *) * new_size);
    if(!new_pqueue) {
        sgx_log_err("resize: malloc");

        return -1;
    }
    memcpy(new_pqueue, pqueue -> node, sizeof(void *) * (pqueue -> size + 1));
    free(pqueue -> node);
    pqueue -> node = new_pqueue;
    pqueue -> max_size = new_size;

    return SGX_OK;
}

static void priority_queue_swap(sgx_priority_queue *pqueue, size_t index_x, size_t index_y)
{
    void *temp;

    // printf("%d %d\n", index_x, index_y);
    // printf("%d %d\n", *(int *)pqueue -> node[index_x], *(int *)pqueue -> node[index_y]);
    temp = pqueue -> node[index_x];
    pqueue -> node[index_x] = pqueue -> node[index_y];
    pqueue -> node[index_y] = temp;
    // printf("%d %d\n", *(int *)pqueue -> node[index_x], *(int *)pqueue -> node[index_y]);
}

static void priority_queue_up(sgx_priority_queue *pqueue, size_t index)
{
    while(index > 1 && pqueue -> cmp(pqueue -> node[index], pqueue -> node[index >> 1])) {
        priority_queue_swap(pqueue, index, index >> 1);
        index >>= 1;
    }
    // priority_queue_show(pqueue);
}

static int priority_queue_cmp(sgx_priority_queue *pqueue, size_t index_x, size_t index_y) {
    if(index_x > pqueue -> size || index_y > pqueue -> size) {
        return 0;
    }
    else {
        return pqueue -> cmp(pqueue -> node[index_x], pqueue -> node[index_y]);
    }
}

static size_t priority_queue_down(sgx_priority_queue *pqueue, size_t index)
{
    size_t size;

    size = pqueue -> size;
    while((index << 1) <= size) {
        size_t index_t;

        index_t = index << 1;
        if(priority_queue_cmp(pqueue, index_t | 1, index_t)) {
            index_t | 1;
        }
        if(priority_queue_cmp(pqueue, index_t, index)) {
            priority_queue_swap(pqueue, index_t, index);
        }
        else {
            break;
        }
        index = index_t;
    }

    return index;
}

int sgx_priority_queue_greater(void *x, void *y)
{
    return *(int *)x > *(int *)y;
}

int sgx_priority_queue_init(sgx_priority_queue *pqueue, sgx_priority_queue_comparator cmp, size_t max_size)
{
    pqueue -> node = (void **)malloc(sizeof(void *) * (max_size + 1));
    if( !pqueue -> node) {
        sgx_log_err("sgx_priority_queue_init: malloc");

        return -1;
    }
    pqueue -> size = 0;
    pqueue -> max_size = max_size;
    pqueue -> cmp = cmp;

    return SGX_OK;
}

int sgx_priority_queue_empty(sgx_priority_queue *pqueue)
{
    return (pqueue -> size) ? 0 : 1;
}

size_t sgx_priority_queue_size(sgx_priority_queue *pqueue)
{
    return pqueue -> size;
}

void *sgx_priority_queue_top(sgx_priority_queue *pqueue)
{
    if(sgx_priority_queue_empty(pqueue)) {
        return NULL;
    }    
    return pqueue -> node[1];
}

int sgx_priority_queue_push(sgx_priority_queue *pqueue, void *value)
{
    /// 扩容
    /// TODO：考虑 * 1.5？
    if(pqueue -> size + 1 == pqueue -> max_size) {
        if(priority_queue_resize(pqueue, pqueue -> max_size << 1) < 0) {
            return -1;
        }
    }
    pqueue -> size ++;
    pqueue -> node[pqueue -> size] = value;
    priority_queue_up(pqueue, pqueue -> size);

    return SGX_OK;
}

int sgx_priority_queue_pop(sgx_priority_queue *pqueue)
{
    if(sgx_priority_queue_empty(pqueue)) {
        return SGX_OK;
    }
    priority_queue_swap(pqueue, 1, pqueue -> size);
    free(pqueue -> node + pqueue -> size);
    pqueue -> size --;
    priority_queue_down(pqueue, 1);
    /// 缩容
    if (pqueue -> size > 0 && pqueue -> size <= ((pqueue -> max_size - 1) >> 2)) {
        if (priority_queue_resize(pqueue, pqueue -> max_size >> 1) < 0) {
            return -1;
        }
    }

    return SGX_OK;
}

int sgx_priority_queue_clear(sgx_priority_queue *pqueue)
{
    while(!sgx_priority_queue_empty(pqueue)) {
        sgx_priority_queue_pop(pqueue);
    }
}