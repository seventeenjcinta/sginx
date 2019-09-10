#include <sgx_config.h>
#include <sgx_core.h>

#define SGX_SLAB_PAGE_MASK   3
#define SGX_SLAB_PAGE        0
#define SGX_SLAB_BIG         1
#define SGX_SLAB_EXACT       2
#define SGX_SLAB_SMALL       3

#if (SGX_PTR_SIZE == 4)

#define SGX_SLAB_PAGE_FREE   0
#define SGX_SLAB_PAGE_BUSY   0xffffffff
#define SGX_SLAB_PAGE_START  0x80000000

#define SGX_SLAB_SHIFT_MASK  0x0000000f
#define SGX_SLAB_MAP_MASK    0xffff0000
#define SGX_SLAB_MAP_SHIFT   16

#define SGX_SLAB_BUSY        0xffffffff

#else /* (SGX_PTR_SIZE == 8) */

#define SGX_SLAB_PAGE_FREE   0
#define SGX_SLAB_PAGE_BUSY   0xffffffffffffffff
#define SGX_SLAB_PAGE_START  0x8000000000000000

#define SGX_SLAB_SHIFT_MASK  0x000000000000000f
#define SGX_SLAB_MAP_MASK    0xffffffff00000000
#define SGX_SLAB_MAP_SHIFT   32

#define SGX_SLAB_BUSY        0xffffffffffffffff

#endif

#define sgx_slab_slots(pool)                                                  \
    (sgx_slab_page_t *) ((u_char *) (pool) + sizeof(sgx_slab_pool_t))

#define sgx_slab_page_type(page)   ((page)->prev & SGX_SLAB_PAGE_MASK)

#define sgx_slab_page_prev(page)                                              \
    (sgx_slab_page_t *) ((page)->prev & ~SGX_SLAB_PAGE_MASK)

#define sgx_slab_page_addr(pool, page)                                        \
    ((((page) - (pool)->pages) << sgx_pagesize_shift)                         \
     + (uintptr_t) (pool)->start)


#if (SGX_DEBUG_MALLOC)

#define sgx_slab_junk(p, size)     sgx_memset(p, 0xA5, size)

#elif (SGX_HAVE_DEBUG_MALLOC)

#define sgx_slab_junk(p, size)                                                \
    if (sgx_debug_malloc)          sgx_memset(p, 0xA5, size)

#else

#define sgx_slab_junk(p, size)

#endif

static sgx_slab_page_t *sgx_slab_alloc_pages(sgx_slab_pool_t *pool, sgx_uint_t pages);
static void sgx_slab_free_pages(sgx_slab_pool_t *pool, sgx_slab_page_t *page, sgx_uint_t pages);
static void sgx_slab_error(sgx_slab_pool_t *pool, sgx_uint_t level, char *text);

static sgx_uint_t  sgx_slab_max_size;
static sgx_uint_t  sgx_slab_exact_size;
static sgx_uint_t  sgx_slab_exact_shift;

void sgx_slab_sizes_init(void)
{
    sgx_slab_max_size = sgx_pagesize / 2;
    sgx_slab_exact_size = sgx_pagesize / (8 * sizeof(uintptr_t));
    for(sgx_uint_t i = sgx_slab_exact_size; i >>= 1; sgx_slab_exact_shift ++){
        /* void */
    }
}

void sgx_slab_init(sgx_slab_pool_t *pool)
{
    u_char *p;
    size_t size;
    sgx_uint_t n;
    sgx_uint_t page_num;
    sgx_int_t m;
    sgx_slab_page_t *slots; /// slot 分级数组
    sgx_slab_page_t *pages;  /// page 管理结构

    pool -> min_size = (size_t)1 << pool -> min_shift;

    /// 跳过ngx_slab_page_t的空间，即跳过slab header
    slots = sgx_slab_slots(pool);
    p = (u_char *) slots;
    size = pool -> end - p;
    sgx_slab_junk(p, size);
    /// 计算可分的级数，page_size为4kb时对应的shift为12，若
    /// 最小可为8B，则shift为3，则对应可分为12-3,即8,16,32,64,
    /// 128,256,512,1024,2048 9个分级。
    /// 从最小 slot 大小到 page 大小之间的分级数
    n = sgx_pagesize_shift - pool -> min_shift;
    for(sgx_uint_t i = 0; i < n; i ++) {
        slots[i].slab = 0;
        slots[i].next = &slots[i];
        slots[i].pre = 0;
    }
    /// 跳过slot分级数组区域
    p += n * sizeof(sgx_slab_page_t);
    pool -> stats = (sgx_slab_status_t *)p;
    sgx_memzero(pool -> status, n * sizeof(sgx_slab_status_t));
    p += n * sizeof(sgx_slab_status_t);
    pool -> pages = (sgx_slab_page_t *)p;
    sgx_memzero(pool -> status, n * sizeof(sgx_slab_page_t));
    size -= n * (sizeof(sgx_slab_page_t) + sizeof(sgx_slab_status_t));
    /// 每一个 page 均对应一个 ngx_slab_page_t 的管理结构
    page_num = (sgx_uint_t)(size / (sgx_pagesize + sizeof(sgx_slab_page_t)));
    pages = pool -> pages;
    pool -> free.slab = 0;
    pool -> free.next = pages;
    pool -> free.prev = 0;

    pages -> slab = page_num;
    pages -> next = &pool -> free;
    pages -> prev = (uintptr_t)&pool -> free;
    /// 内存对齐，在膜 sgx_pagesize 意义下向上取整
    pool -> start = sgx_align_ptr(p + page_num * sizeof(sgx_slab_page_t), sgx_pagesize);
    m = pagenum - (pool -> end - pool -> start) / sgx_pagesize;
    if(m > 0) {
        pagenum -= m;
        pages -> slab = pages;
    }
    pool -> last = pool -> pages + pagenum;
    pool -> pfree = pagenum;
    pool -> log_nomem = 1;
    pool -> log_ctx = &pool -> zero;
    pool -> zero = '\0';
}

void sgx_slab_alloc_locked(sgx_slab_pool_t *pool, size_t size)
{
    uintptr_t p;
    uintptr_t m;
    uintptr_t mask;
    uintptr_t *bitmap;
    sgx_uint_t n;
    sgx_uint_t slot_id;
    sgx_uint_t map;
    sgx_uint_t shift;
    sgx_slab_page_t *pages;
    sgx_slab_page_t *prev;
    sgx_slab_page_t *slots;

    /// 请求的size大于最大的slot的大小，直接以页的形式分配空间
    if(size > sgx_slab_max_size) {
        ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, ngx_cycle -> log, 0, "slab alloc: %uz", size);
        /// 获取 page
        pages = sgx_slab_alloc_pages(pool, (size >> sgx_pagesize_shift) + ((size % sgx_pagesize) ? 1 : 0));
        if(pages) {
            /// 计算具体位置
            p = sgx_slab_page_addr(pool, pages);
            /// p = (page - pool->pages) << ngx_pagesize_shift;
            /// p += (uintptr_t) pool->start;
        }
        else {
            p = 0;
        }

        goto done;
    }    
    /// 请求的 size 小于等于 2048，可用 slot
    /// 获取 slot 编号
    if(size > pool -> min_size) {
        size_t s;
        
        shift = 1;
        s = size - 1;
        while(s) {
            shift ++;
            s >>= 1;
        }
        slot_id = shift - pool -> min_shift;
    }
    else {
        shift = pool -> min_shift;
        slot_id = 0;
    }
    pool -> status[slot_id].reqs ++;
    ngx_log_debug2(NGX_LOG_DEBUG_ALLOC, ngx_cycle->log, 0, "slab alloc: %uz slot: %ui", size, slot);
    /// 跳过 ngx_slab_page_t 的空间，即跳过 slab header
    slots = sgx_slab_slots(pool);
    page = slots[slot_id].next;
    if(page -> next != page) {
        if(shift < sgx_slab_exact_shift) {
            bitmap = (uintptr_t *) sgx_slab_page_addr(pool, page);
            map = (sgx_pagesize >> shift) / (8 * sizeof(uintptr_t));
            for(sgx_uint_t i = 0; i < map; i ++) {
                if(bitmap[i] != SGX_SLAB_BUSY) {
                    for(sgx_uint_t j = 1, k = 0; j; j <<= 1, k ++) {
                        if(bitmap[i] & j) {
                            continue;
                        }
                        bitmap[i] |= j;
                        k = (i * 8 * sizefo(uintptr_t) + k) << shift;
                        p = (uintptr_t)bitmap + i;
                        pool -> status[slot].used ++;
                        if(bitmap[i] == SGX_SLAB_BUSY) {
                            i ++;
                            while(i < map) {
                                if(bitmap[i] != SGX_SLAB_BUSY) {
                                    goto done;
                                }
                            }
                            prev = sgx_slab_page_prev(page);
                            prev -> next = page -> next;
                            page -> next -> prev = page -> prev;
                            page -> next = NULL;
                            page -> prev = SGX_SLAB_SMALL;
                        }
                        goto done;
                    }
                }
            }
        }
        else if(shift == sgx_slab_exact_shift) {
            for(sgx_uint_t i= 1； = )
        }
    }
}