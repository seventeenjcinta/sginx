#ifndef _NGX_SLAB_H_INCLUDED_
#define _NGX_SLAB_H_INCLUDED_

#include <sgx_config.h>
#include <sgx_core.h>

typedef struct sgx_slab_page_s  sgx_slab_page_t;

struct sgx_slab_page_s
{
    uintptr_t slab;
    sgx_slab_page_t *next;
    uintptr_t prev;
};

typedef struct
{
    sgx_uint_t toal;
    sgx_uint_t used;
    sgx_uint_t reqs;
} sgx_slab_status_t;

typedef struct 
{
    sgx_shmtx_sh_t lock;
    size_t min_size;            /// slot 块大小的下限, 而且这个下限必须大于 8
    size_t min_shift;           /// 最小左移距离, 与上面的联合使用, 必须大于 3
    sgx_slab_page_t *pages;     /// 指向 m_page 数组
    sgx_slab_page_t *last;      /// 指向 slab page 管理结构的开始位置
    sgx_slab_page_t free;       /// 管理free的页面, 空闲的 slab page 管理结构链表
    sgx_slab_status_t *status;  /// 提供共享内存的状态信息
    sgx_uint_t pfree;
    u_char *start;              /// 数据区的起始地址
    u_char *end;                /// 数据区的结束地址
    sgx_shmtx_t mutex;          /// 共享内存锁
    u_char *log_ctx;
    u_char zero;
    unsigned log_nomem:1;
    void *data;
    void *addr;
}sgx_slab_pool_t;

void sgx_slab_sizes_init(void);
void sgx_slab_init(sgx_slab_pool_t *pool);
void *sgx_slab_alloc(sgx_slab_pool_t *pool, size_t size);
void *sgx_slab_alloc_locked(sgx_slab_pool_t *pool, size_t size);
void *sgx_slab_calloc(sgx_slab_pool_t *pool, size_t size);
void *sgx_slab_calloc_locked(sgx_slab_pool_t *pool, size_t size);
void sgx_slab_free(sgx_slab_pool_t *pool, void *p);
void sgx_slab_free_locked(sgx_slab_pool_t *pool, void *p);

#endif /* _NGX_SLAB_H_INCLUDED_ */