// #ifndef THREAD_POOL_H__
// #define THREAD_POOL_H__
#include <pthread.h>

typedef struct sgx_work_list_s
{
    void *(*routine)(void *);   /// 任务函数
    void *arg;                  /// 任务函数参数
    struct sgx_work_list_s *next;
}sgx_work_list;

struct sgx_thread_pool
{
    int shutdown;                    /// 线程池是否被销毁
    int max_num;                    /// 最大线程数
    pthread_t *thread_id;           /// 线程 ID 数组
    sgx_work_list *now;             /// 线程链表
    pthread_mutex_t lock;           /// 锁
    pthread_cond_t ready;           /// 条件变量
};

/// 0 - 成功
int sgx_thread_pool_create(int sgx_max_num);
void sgx_thread_pool_destroy();
int sgx_thread_pool_add_work(void *(*routine)(void *), void *arg);
// #endif