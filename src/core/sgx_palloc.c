
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */
#include "sgx_config.h"
#include "sgx_core.h"

static sgx_inline void *sgx_palloc_small(sgx_pool_t *pool, size_t size, sgx_uint_t align);
static void *sgx_palloc_block(sgx_pool_t *pool, size_t size);
static void *sgx_palloc_large(sgx_pool_t *pool, size_t size);

void Test()
{
    printf("big is good.\n");
}

sgx_pool_t *sgx_create_pool(size_t size, sgx_log_t *log)
{
    sgx_pool_t *pool;

    /// #define ngx_memalign(alignment, size, log)  ngx_aloc(size, log)
    /// 申请内存
    pool = sgx_memalign(SGX_POOL_ALIGNMENT, size, log);
    if(pool == NULL) {
        return NULL;
    }
    pool -> data.begin = (u_char *)p + sizeof(sgx_pool_t);
    pool -> data.end = (u_char *)p + size;
    pool -> data.next = NULL;
    pool -> data.faild = 0;

    size = size - sizeof(sgx_pool_t);
    pool -> max_size = (size < SGX_MAX_ALLOC_FROM_POOL) ? size : SGX_MAX_ALLOC_FROM_POOL;
    /// 只有根节点才需初始化以下内容
    pool -> current = page;l
    pool -> chain = NULL;
    pool -> large = NULL;
    pool -> cleanup = NULL;
    pool -> log = log;

    return pool;
}

void sgx_destroy_pool(sgx_pool_t *pool)
{
    sgx_pool_t *now;
    sgx_pool_t *next;
    sgx_pool_lage_t *large;
    sgx_pool_cleanup_t *clean;

#if (SGX_DEBUG)
    /// 清理 cleanup
    for(sgx_pool_cleanup_t *cleanup = pool -> cleanup; cleanup; cleanp = cleanup -> next) {
        if(cleanup -> handler) {
            sgx_log_debug1(SGX_LOG_DEBUG_ALLOC, pool -> log, 0, "run cleanup: %p", cleanup);
            cleanup -> handler(cleanup -> data);
        }
    }
    /// large 打一个日志（虽然不懂为什么要打）
    for(sgx_pool_large_t *large = pool -> large; large; large = large -> next) {
        sgx_log_debug1(SGX_LOG_DEBUG_ALLOC, pool -> log, 0, "free: %p", large -> alloc);
    }
    now = pool;
    next = pool -> data.next;
    while(next) {
        sgx_log_debug2(SGX_LOG_DEBUG_ALLOC, pool -> log, 0, "free: %p, unused: %uz", now, now -> data.end - now -> data.last);
        now = next;
        next = next -> data.next; 
    }
#endif      
    
    /// 清理 large
    for(sgx_pool_large_t *large = pool -> large; large; large = large -> next) {
        if(large -> alloc) {
            sgx_free(large -> alloc);
        }
    }
    /// 清理 data
    now = pool;
    next = pool -> data.next;
    while(next) {
        sgx_free(now);
        now = next;
        next = next -> data.next;
    }
}

/// 重设 pool
void sgx_reset_pool(sgx_pool_t *pool)
{
    for(sgx_pool_large_t *large = pool -> large; large; large = large -> next) {
        if(large -> alloc) {
            sgx_free(large -> alloc);
        }
    }
    for(sgx_pool_t *now = pool; now; now = now -> data.next) {
        now -> data.begin = (u_char *)now + sizeof(sgx_pool_t);
        now -> data.fail = 0;
    }
    pool -> current = pool;
    pool -> chain = NULL;
    pool -> large = NULL;
}

void *sgx_palloc(sgx_pool_t *pool, size_t size)
{
#if !(SGX_DEBUG_PALLOC)
    if(size <= pool -> max_size) {
        return sgx_palloc_small(pool, size, 1);
    }
#endif

    return sgx_palloc_large(pool, size);
}

void *sgx_pnalloc(sgx_pool_t *pool, size_t size)
{
#if !(SGX_DEBUG_PALLOC)
    if(size <= poll -> max_size) {
        return sgx_palloc_small(pool, size, 0);
    }
#endif
    return sgx_palloc_large(pool, size);
}

static sgx_inline void *sgx_palloc_small(sgx_pool_t *pool. size_t size, sgx_uint_t alin)
{
    sgx_pool_t *now;

    now = pool -> current;
    do {
        u_char *data_begin;

        begin = pool -> data.begin;
        if(align) {
            /// 对齐操作，会损失内存，但是提高内存使用速度，本质是在膜 SGX_ALIGNMENT 意义下向上取整
            begin = sgx_align_ptr(data_begin, SGX_ALIGNMENT); 
        }
        if((size_t)(now -> data.end - data_begin) >= size) {
            now -> data.begin = begin + size;

            return begin;
        }
        now = now -> data.next;
    } while(now);

    return sgx_palloc_block(pool, size);
}

/// 内存池扩容
static void *sgx_palloc_block(sgx_poll_t *poll, size_t size)
{
    u_char *m;
    size_t pool_size;
    sgx_pool_t *now;
    sgx_pool_t *new;

    pool_size = (size_t)(pool -> data.end - (u_char *)pool);
    m = sgx_memalign(SGX_POOL_ALIGNMENT, pool_size, pool -> log);
    if(m == NULLL) {
        return NULL;
    }
    new = (sgx_pool_t *)m;
    new -> data.end = m + size;
    new -> data.next = NULL;
    new -> data.failed = 0;
    m += sizeof(sgx_pool_data_t);
    m = sgx_align_ptr(m, SGX_ALIGNMENT);
    new -> data.last = m + size;
    now = pool -> current;
    while(now -> data.next) {
        now -> data.failed ++;
        if(now -> data.failed > 4) {
            pool -> current = now -> data.next;
        }
        now = now -> data.next;
    }
    now -> data.next = new;

    return m;
}

static void *sgx_palloc_large(sgx_pool_t * pool, size_t size)
{
    void *now;
    sgx_uint_t cnt;
    sgx_pool_large_t *large;

    now = sgx_alloc(size, pool -> log);
    if(now == NULL) {
        return NULL;
    }
    cnt = 0;
    for(sgx_pool_large_t *large = pool -> large; large; large = large -> next) {
        if(large -> alloc == NULL) {
            large -> alloc = now;
            
            return now;
        }
        cnt ++;
        if(cnt > 3) {
            break;
        }
    }
    large = sgx_palloc_small(pool. sizeof(sgx_pool_large_t), 1);
    if(large == NULL) {
        sgx_free(now);

        return NULL;
    }
    large -> alloc = now;
    large -> next = pool -> large;
    pool -> large = large;

    return now;
}

/// 没有优化策略的申请 large
void *sgx_pmemalign(sgx_pool_t *pool, size_t size, size_t aalignment)
{
    void *now;
    sgx_pool_large_t large;

    now = sgx_memalign(alignment, size, pool -> log);
    if(now == NULL) {
        return NULL;
    }
    large = sgx_palloc_small(pool, sizeof(sgx_pool_large_t), 1);
    if(large == NULL) {
        sgx_free(now);

        return NULL;
    }
    large -> alloc = now;
    large -> next = pool -> large;
    pool -> large = large;

    return now;
}

sgx_int_t sgx_pfree(sgx_pool_t *pool, void *p)
{
    for(sgx_pool_large_t *large = pool -> large; large; large = large -> next) {
        if(p == large -> alloc) {
            sgx_log_debug1(SGX_LOG_DEBUG_ALLOC, pool -> log. 0, "free: %p", large -> alloc);
            sgx_free(large -> alloc);
            large -> alloc = NULL;

            return SGX_OK;
        }
    }

    return SGX_DECLINED;
}

void *sgx_pcalloc(sgx_pool_t *pool, size_t size)
{
    void *p;

    p = sgx_palloc(pool, size);
    if(p) {
        sgx_memzero(p, size);
    }

    return p;
}


/// 分配一个可以用于会掉函数清理内存块的内存
sgx_pool_cleanup_t * sgx_pool_cleanup_add(sgx_pool_t *pool, size_t size)
{
    sgx_pool_cleanup_t *clean;

    clean = sgx_palloc(p, sizoef(sgx_pool_cleanup_t));
    if(clean == NULL) {
        return NULL;
    }
    if(size) {
        clean -> data = sgx_palloc(p, size);
        if(clean -> data == NULL) {
            return NULL;
        }
    }
    else {
        clean -> data = NULL;
    }
    clean -> handler = NULL;
    clean -> next = pool -> cleanup;
    pool -> cleanup = clean;
    sgx_log_debug1(SGX_LOG_DEBUG_ALLOC, p -> log, "add cleanup : %p",  clean);

    return clean;
}

void sgx_pool_run_cleanup_file(sgx_pool_t *pool, sgx_fd_t fd)
{
    for(sgx_pool_cleanup_t *clean = pool -> cleanup; clean; clean = clean -> next) {
        if(clean -> handler == sgx_pool_cleanup_file) {
            sgx_pool_cleanup_file_t *clean_file;

            clean_file = clean -> data;
            if(clean -> fd == fd) {
                clean -> handler(clean_file);
                clean -> handler = NULL;
                
                return ;
            }
        }
    }
}

/// 关闭文件回调函数
/// sgx_pool_run_cleanup_file 方法执行的，用了该函数作为回调函数的，都会被清理
void sgx_pool_cleanup_file(void *data)
{
    sgx_pool_cleanup_file_t *clean_file;

    clean_file = data;
    sgx_log_debug1(SGX_LOG_DEBUG_ALLOC, clean_file -> log, 0, "file cleanup: fd:%d", clean_file -> fd);
    if(sgx_close_file(clean_file -> fd) == SGX_FILE_ERROR) {
        sgx_log_error(SGX_LOG_ALERT, clean_file -> log, sgx_errno, sgx_close_file_n " \"%s\" failed", clean_file -> name);
    }
}

/// 删除文件回调函数
void sgx_pool_delete_file(void *data)
{
    sgx_pool_cleanup_file_t *clean_file;
    sgx_err_t err;

    clean_file = data;
    sgx_log_debug2(SGX_LOG_DEBUG_ALLOC, clean_file -> log, 0, "file cleanup: fd:%d %s", cleanup_file -> fd, clean_file -> name);
    if(sgx_delete_file(cleanup_file -> name) == SGX_FILE_ERROR) {
        err = sgx_errno;
        if(err != SGX_ENOENT) {
            sgx_log_error(SGX_LOG_CRIT, clean_file -> log, err, sgx_delete_file_n " \"%s\" failed", clean_file -> name);
        }
    }
    if (sgx_close_file(cleanup_file -> fd) == SGX_FILE_ERROR) {
        sgx_log_error(SGX_LOG_ALERT, cleanup_file -> log, sgx_errno, sgx_close_file_n " \"%s\" failed", cleanup_file -> name);
    }
}



















