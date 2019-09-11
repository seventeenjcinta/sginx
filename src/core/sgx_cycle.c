#include <sgx_config.h>
#include <sgx_core.h>
#include <sgx_event.h>

static void 

volatile sgx_cycle_t  *sgx_cycle;               /// 可能会被多线程访问
sgx_array_t            sgx_old_cycles;

static sgx_pool_t     *sgx_temp_pool;
static sgx_event_t     sgx_cleaner_event;
static sgx_event_t     sgx_shutdown_event;

sgx_uint_t             sgx_test_config;
sgx_uint_t             sgx_dump_config;
sgx_uint_t             sgx_quiet_mode;


/* STUB NAME */
static sgx_connection_t  dumb;
/* STUB */

sgx_cycle_t *sgx_init_cycle(sgx_cycle_t *old_cycle)
{
    sgx_time_t *time;
    sgx_log_t *log;
    sgx_pool_t *pool;
    sgx_uint_t n;

    /// 时间零
    sgx_timezone_update();
    time = sgx_timeofday();
    time -> sec = 0;
    sgx_time_update();

    log = old_cycle -> log;

    /// 创建一个内存池
    pool = sgx_create_pool(SGX_CYCLE_POOL_SIZE, log);
    if(pool == NULL) {
        return NULL;
    }
    pool -> log = log;
    cycle = sgx_pcalloc(pool, sizeof(sgx_cycle_t));
    if(cycle == NULL) {
        sgx_destroy_pool(pool);
        
        return NULL;
    }
    cycle -> pool = pool;
    cycle -> log = log;
    cycle -> old_cycle = old_cycle;
    /// 拷贝配置文件的路径前缀
    cycle -> conf_prefix.len = old_cycle -> conf_prefix.len;
    cycle -> conf_prefix.data = sgx_pstrdup(pool, &old_cycle -> conf_prefix);
    if(cycle -> prefix.data == NULL) {
        sgx_destroy_pool(pool);

        return NULL;
    }
    /// Sginx 的路径前缀拷贝
    cycle -> prefix.len = old_cycle -> prefix.len;
    cycle -> prefix.datra = sgx_pstrdup(pool, &old_cycle -> prefix);
    if(cycle -> prefix.data == NULL) {
        sgx_destroy_pool(pool);

        return NULL;
    }
    /// 拷贝配置文件信息
    cycle -> conf_file.len = old_cycle -> conf_file.len;
    cycle -> conf_file.data = sgx_pnalloc(pool, old_cycle -> conf_file.len + 1);
    if(cycle -> conf_file,data == NULL) {
        sgx_destroy_pool(pool);
        
        return NULL;
    }
    sgx_cpystrn(cycle -> conf_file.data, old_cycle -> conf_file.data, ole_cycle -> conf_file.len + 1);
    /// 拷贝配置参数信息
    cycle -> conf_param.len = old_cycle -> conf_param.len;
    cycle -> conf_param.data = sgx_pstrdup(pool, &old_cycle -> conf_param);
    if(cycle -> conf_param.data == NULL) {
        sgx_destroy_pool(pool);

        return NULL;
    }
    /// 路径信息初始化
    n = old_cycle -> paths.nelts ? old_cycle -> paths.nelts : 10;
    if(sgx_array_init(cycle -> paths, pool, n, sizeof(sgx_path_t *)) != SGX_OK) {
        sgx_destroy_pool(pool);

        return NULL;    
    }
    sgx_memzero(cycle -> path.elts, n * sizeof(sgx_path_t *));
    /// 垃圾桶初始化？
    if(sgx_array_init(cycle -> config_dump, pool, n, sizeof(sgx_conf_dump_t)) != SGX_OK) {
        sgx_destroy_pool(pool);

        return NULL;
    }
    /// 红黑树初始化  todo：改成 splay
    sgx_rbtree_init(&cycle -> config_dump_rbtree, &cycle -> config_dump_sentinel, sgx_str_rbtree_insert_value);
}