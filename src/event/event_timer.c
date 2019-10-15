#include <sys/time.h>
#include "sgx_common.h"
#include "sgx_core.h"
#include "event/event_timer.h"

extern sgx_priority_queue sgx_timer_Q;
extern size_t sgx_current_msec;

static int timer_cmp(void *timer_x, void *timer_y)
{
    sgx_timer_node *x = (sgx_timer_node *)timer_x;
    sgx_timer_node *y = (sgx_timer_node *)timer_y;

    return x -> key < y -> key;
}

static void timer_update()
{
    /// 好像只有一个线程会调用
    struct timeval value;
    int rc;

    rc = gettimeofday(&value, NULL);
    if(rc != 0) {
        sgx_log_err("timer_update: call gettineofday()");
    }
    sgx_current_msec = value.tv_sec * 1000 + value.tv_usec / 1000;
    sgx_log_info("timer_update: time = %zu", sgx_current_msec);
}

int sgx_timer_init()
{
    int result;

    result = sgx_priority_queue_init(&sgx_timer_Q, timer_cmp, SGX_PRIORITY_QUEUE_DEFAULT_SIZE);
    if(result != SGX_OK) {
        sgx_log_err("sgx_timer_init: call sgx_priority_queue_init()");
    }
    timer_update();

    return SGX_OK;
}

/// 第一个未被删除的时间
int sgx_timer_find()
{
    sgx_timer_node *node;
    int time;

    time = SGX_TIMER_INFINITE;
    while(!sgx_priority_queue_empty(&sgx_timer_Q)) {
        timer_update();
        node = (sgx_timer_node *)sgx_priority_queue_top(&sgx_timer_Q);
        if(node == NULL) {
            sgx_log_err("sgx_timer_find: call sgx_priority_queue_top()");
        }
        if(node -> del) {
            int result; 
            
            result = sgx_priority_queue_pop(&sgx_timer_Q);
            if(result != SGX_OK) {
                sgx_log_err("sgx_timer_find: call sgx_priority_queue_pop()");
            }
            continue;
        }
        time = (int) (node -> key - sgx_current_msec);
        time = SGX_MAX(time, 0);
        break;
    }

    return time;
}

void sgx_timer_add(sgx_http_request *request, size_t timeout, timer_handler_pt handler)
{
    int result;
    sgx_timer_node *node;

    node = (sgx_timer_node *)malloc(sizeof(sgx_timer_node));
    if(node == NULL) {
        sgx_log_err("sgx_timer_add: call malloc()");
    }
    timer_update();
    node -> key = sgx_current_msec + timeout;
    node -> del = 0;
    node -> handler = handler;
    node -> request = req;
    request -> timer = node;
    result = sgx_priority_queue_push(&sgx_timer_Q, node);
    if(result != SGX_OK) {
        sgx_log_err("sgx_timer_add: sgx_priority_queue_push()");
    }
}

void sgx_timer_del(sgx_http_request *request)
{
    sgx_timer_node *node;

    timer_update();
    node = request -> timer;
    if(node == NUL) {
        sgx_log_err("sgx_timer_del: request -> timer is NULL");
    }
    node -> del = 1;
}

/// 时间到期
void sgx_handle_expire_timers()
{
    sgx_timer_node *node;

    while(!sgx_priority_queue_empty(&sgx_timer_Q)) {
        int result;

        timer_update();
        node = (sgx_timer_node *)sgx_priority_queue_top(&sgx_timer_Q);
        if(node == NULL) {
            sgx_log_err("call sgx_priority_queue_top()");
        }
        if(node -> del) {
            result = sgx_priority_queue_pop(&sgx_timer_Q);
            if(result != SGX_OK) {
                sgx_log_err("call sgx_priority_queue_pop()");
            }
            continue;
        }
        if(node -> key > sgx_current_msec) {
            return ;
        }  
        if(node -> handler) {
            node -> handler(node -> request);
        }
        result = sgx_priority_queue_pop(&sgx_timer_Q);
        if(result != SGX_OK) {
            sgx_log_err("call sgx_priority_queue_pop()");
        }
    }
}