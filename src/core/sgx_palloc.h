/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


// #ifndef _SGX_PALLOC_H_INCLUDED_
// #define _SGX_PALLOC_H_INCLUDED_


#include "sgx_config.h"
#include "sgx_core.h"

#define SGX_POOL_ALIGNMENT 16
#define SGX_MAX_ALLOC_FROM_POOL (sgx_pagesizse - 1)
#define SGX_DEFAULT_POOL_SIZE (16 * 1024)
#define SGX_MIN_POOL_SIZE sgx_align((sizeof(sgx_pool_t) + 2 * sizeof(sgx_pool_large_t)), SGX_POOL_ALIGNMENT)

typedef void (*sgx_pool_cleanup_pt)(void *data);
typedef struct sgx_pool_cleanup_s sgx_pool_cleanup_t;
typedef struct sgx_pool_large_s sgx_pool_larges_t;

struct sgx_pool_cleanup_s 
{
    sgx_pool_cleanup_pt handler;    /// 清理的回调函数
    void *data;                     /// 指向存储区的数据地址
    sgx_pool_cleanup_t *next;       /// 下一个 sgx_pool_cleanup_t
};

struct sgx_pool_large_s
{
    sgx_pool_larges_t *next;        /// 下一个存储地址
    void *alloc;                    /// 数据块指针地址
};

typedef struct 
{
    u_char *begin;      /// 内存池中未使用内存的开始节点地址
    u_char *end;        /// 内存池的结束地址
    sgx_pool_t *next;   /// 下一个内存池地址
    sgx_uint_t failed;  /// 失败次数
}sgx_pool_data_t;

struct sgx_pool_s
{
    sgx_log_t *log;                 /// 日志
    sgx_pool_data_t data;           /// 数据区域    
    size_t max_size;                /// 最大可分配内存
    sgx_pool_t *current;            /// 当前指向的内存池地址
    sgx_chain_t *chain;             /// 缓冲区
    sgx_pool_large_t *large;        /// 大数据链表
    sgx_pool_cleanup_t *cleanup;    /// 自定义回调函数
};

typedef struct {
    sgx_fd_t fd;
    u_char *name;
    sgx_log_t *log;
} sgx_pool_cleanup_file_t;

sgx_pool_t *sgx_create_pool(size_t size, sgx_log_t *log);
void sgx_destroy_pool(sgx_pool_t *pool);
void sgx_reset_pool(sgx_pool_t *pool);
void *sgx_palloc(sgx_pool_t *pool, size_t size);
void *sgx_pnalloc(sgx_pool_t *pool, size_t size);   /// 不带内存对齐
void *sgx_pcalloc(ngx_pool_t *pool, size_t size);   /// 附带 memsetzero
void *sgx_pmemalign(sgx_pool_t *pool, size_t size, size_t alignment);
sgx_int_t sgx_pfree(Sgx_pool_t *pool, void *p);


sgx_pool_cleanup_t *sgx_pool_cleanup_add(sgx_pool_t *p, size_t size);
void sgx_pool_run_cleanup_file(sgx_pool_t *p, sgx_fd_t fd);
void sgx_pool_cleanup_file(void *data);
void sgx_pool_delete_file(void *data);

// #endif /* _SGX_PALLOC_H_INCLUDED_ */