/// 定时事件
#ifndef EVENT_TIMER_H
#define EVENT_TIMER_H

#include "sgx_priority_queue.h"
#include "http_request.h"

#define SGX_TIMER_INFINITE -1
#define SGX_TIMEOUT_DEFAULT 500

typedef int (*timer_handler_pt)(sgx_http_request *request);

typedef struct 
{
    size_t key;
    int del;        /// 删除标记，如果远程客户端首先关闭套接字，则将 del 置为 1 
    timer_handler_pt handler;
    sgx_http_request *request;
}sgx_timer_node;

extern sgx_priority_queue sgx_timer_Q;
extern size_t sgx_current_msec;

int sgx_timer_init();
int sgx_timer_find();
void sgx_timer_add(sgx_http_request *request, size_t timeout, timer_handler_pt handler);
void sgx_timer_del(sgx_http_request *request);
void sgx_handle_expire_timers();

#endif