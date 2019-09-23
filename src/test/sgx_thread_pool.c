#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "sgx_thread_pool.h"

static struct sgx_thread_pool *sgx_tpool = NULL;

static void *sgx_thread_routine(void *arg)
{
    sgx_work_list *sgx_work;

    while(1) {
        pthread_mutex_lock(&sgx_tpool -> lock);
        while(!sgx_tpool -> now && !sgx_tpool -> shutdown) {
            pthread_cond_wait(&sgx_tpool -> ready, &sgx_tpool -> lock);
        }
        if(sgx_tpool -> shutdown) {
            pthread_mutex_unlock(&sgx_tpool -> lock);
            pthread_exit(NULL);
        }
        sgx_work = sgx_tpool -> now;
        sgx_tpool -> now = sgx_tpool -> now -> next;
        pthread_mutex_unlock(&sgx_tpool -> lock);
        sgx_work -> routine(sgx_work -> arg);
        free(sgx_work); 
    }

    return NULL;
}

int sgx_thread_pool_create(int sgx_max_num)
{
    sgx_tpool = calloc(1, sizeof(struct sgx_thread_pool));
    if(!sgx_tpool) {
        printf("%s: calloc failed\n", __FUNCTION__);
        exit(1);
    }
    sgx_tpool -> max_num = sgx_max_num;
    sgx_tpool -> shutdown = 0;
    sgx_tpool -> now = NULL;
    if(pthread_mutex_init(&sgx_tpool -> lock, NULL) != 0) {
        printf("%s: pthread_mutex_init faild, errno: %d, error: %s\n", __FUNCTION__, errno, strerror(errno));
        exit(1);
    }
    if(pthread_cond_init(&sgx_tpool -> ready, NULL) != 0) {
        printf("%s: pthread_cond_init faild, errno: %d, error: %s\n", __FUNCTION__, errno, strerror(errno));
        exit(1);
    }
    sgx_tpool -> thread_id = calloc(sgx_max_num, sizeof(struct sgx_thread_pool));
    if(!sgx_tpool -> thread_id) {
        printf("%s: calloc failed\n", __FUNCTION__);
        exit(1);
    }
    for(int i = 0; i < sgx_max_num; i ++) {
        if(pthread_create(&sgx_tpool -> thread_id[i], NULL, sgx_thread_routine, NULL) != 0) {
            printf("%s: pthread_create failed, errno: %d, error: %s\n", __FUNCTION__, errno, strerror(errno));
            exit(1);
        }
    }

    return 0;
}

void sgx_thread_pool_destroy()
{
    if(sgx_tpool -> shutdown) {
        return ;
    }
    sgx_tpool -> shutdown = 1;
    /// 通知所有正在等待的线程
    pthread_mutex_lock(&sgx_tpool -> lock);
    pthread_cond_broadcast(&sgx_tpool -> ready);
    pthread_mutex_unlock(&sgx_tpool -> lock);
    for(int i = 0; i < sgx_tpool -> max_num; i ++) {
        pthread_join(sgx_tpool -> thread_id[i], NULL);
    }
    free(sgx_tpool -> thread_id);
    while(sgx_tpool -> now) {
        sgx_work_list *work;

        work = sgx_tpool -> now;
        sgx_tpool -> now = sgx_tpool -> now -> next;
        free(work);
    }
    pthread_mutex_destroy(&sgx_tpool -> lock);
    pthread_cond_destroy(&sgx_tpool -> ready);
    free(sgx_tpool);
}

int sgx_thread_pool_add_work(void *(*sgx_routine)(void *), void *sgx_arg)
{
    sgx_work_list *sgx_work;
    sgx_work_list *sgx_now;
    
    if(!sgx_routine) {
        printf("%s: Invalid argument\n", __FUNCTION__);

        return -1;
    }
    sgx_work = malloc(sizeof(sgx_work_list));
    if(!sgx_work) {
        printf("%s: malloc failed\n", __FUNCTION__);
        
        return -1;
    }
    sgx_work -> routine = sgx_routine;
    sgx_work -> arg = sgx_arg;
    sgx_work -> next = NULL;
    pthread_mutex_lock(&sgx_tpool -> lock);
    sgx_now = sgx_tpool -> now;
    if(!sgx_now) {
        sgx_tpool -> now = sgx_work;
    }
    else {
        while(sgx_now -> next) {
            sgx_now = sgx_now -> next;
        }
        sgx_now -> next = sgx_work;
    }
    pthread_cond_signal(&sgx_tpool -> ready);
    pthread_mutex_unlock(&sgx_tpool -> lock);

    return 0;
}
