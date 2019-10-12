#ifndef SGX_EPOLL_H
#define SGX_EPOLL_H

#include <sys/epoll.h>
#define SGX_MAX_EVENTS (1024)

int sgx_epool_create(int flags);
void sgx_epoll_add(int epoll_fd, int fs, struct epool_event *event);
void sgx_epoll_mod(int epoll_fd, int fs, struct epool_event *event);
void sgx_epoll_del(int epoll_fd, int fs, struct epool_event *event);
int sgx_epool_wait(int epoll_fd, struct epool_event *events, int max_events, int timeout);

#endif