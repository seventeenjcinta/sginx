#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include "sgx_log.h"

#define SGX_PRIORITY_QUEUE_DEFAULT_SIZE 10

typedef int (*sgx_priority_queue_comparator)(void *qx, void *qy);

typedef struct {
    void **node;
    size_t size;
    size_t max_size;
    sgx_priority_queue_comparator cmp;
} sgx_priority_queue;

int sgx_priority_queue_init(sgx_priority_queue *pqueue, sgx_priority_queue_comparator cmp, size_t max_size);
int sgx_priority_queue_empty(sgx_priority_queue *pqueue);
size_t sgx_priority_queue_size(sgx_priority_queue *pqueue);
void *sgx_priority_queue_top(sgx_priority_queue *pqueue);
int sgx_priority_queue_pop(sgx_priority_queue *pqueue);
int sgx_priority_queue_push(sgx_priority_queue *pqueue, void *value);
int sgx_priority_queue_greater(void *x, void *y);
int sgx_priority_queue_clear(sgx_priority_queue *pqueue);

#endif