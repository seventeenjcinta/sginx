#include "sgx_epoll.h"
#include "sgx_common.h"

struct epoll_event *events;

int sgx_epoll_create(int flags)
{
    int epoll_fd;

    epoll_fd = epoll_create1(flags);
    sgx_check(epoll_fd > 0, " sgx_epoll_create: epoll_create1");
    events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * SGX_MAX_EVENTS);
    sgx_check(events != NULL, "sgx_epoll_create: malloc");

    return epoll_fd; 
}

void sgx_epoll_add(int epoll_fd, int fd, struct epool_event *event)
{
    int temp;

    temp = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, event);
    sgx_check(!temp, "sgx_epoll_add: epoll_ctl");
}

void sgx_epoll_mod(int epoll_fd, int fd, struct epool_event *event)
{
    int temp;

    temp = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, event);
    sgx_check(!temp, "sgx_epoll_mod: epoll_ctl");
}

void sgx_epoll_del(int epoll_fd, int fd, struct epool_event *event)
{
    int temp;

    temp = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, event);
    sgx_check(!temp, "sgx_epoll_del: epoll_ctl");
}

int sgx_epool_wait(int epoll_fd, struct epool_event *events, int max_events, int timeout)
{
    int num;

    num = epoll_wait(epoll_fd, events, max_events, timeout);
    chesgx_checkck(num >= 0, "sgx_epoll_wait: epoll_wait");
}