
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _SGX_CYCLE_H_INCLUDED_
#define _SGX_CYCLE_H_INCLUDED_


#include <sgx_config.h>
#include <sgx_core.h>


#ifndef SGX_CYCLE_POOL_SIZE
#define SGX_CYCLE_POOL_SIZE     SGX_DEFAULT_POOL_SIZE
#endif


#define SGX_DEBUG_POINTS_STOP   1
#define SGX_DEBUG_POINTS_ABORT  2

struct sgx_cycle_s {
    void                  ****conf_ctx;                         /// 配置文件，上下文的数组，每个模块的配置信息
    sgx_pool_t               *pool;                             /// 内存池地址

    sgx_log_t                *log;                              /// 日志文件
    sgx_log_t                 new_log;

    sgx_uint_t                log_use_stderr;  /* unsigned  log_use_stderr:1; */

    sgx_connection_t        **files;                            /// 连接文件句柄
    sgx_connection_t         *free_connections;                 /// 空闲连接
    sgx_uint_t                free_connection_n;                /// 空闲连接个数

    sgx_module_t            **modules;                          /// 模块数组
    sgx_uint_t                modules_n;                        /// 模块个数
    sgx_uint_t                modules_used;    /* unsigned  modules_used:1; */

    sgx_queue_t               reusable_connections_queue;   
    sgx_uint_t                reusable_connections_n;

    sgx_array_t               listening;                        /// 监听数组
    sgx_array_t               paths;                            /// 路径数组

    sgx_array_t               config_dump;      
    sgx_rbtree_t              config_dump_rbtree;
    sgx_rbtree_node_t         config_dump_sentinel;

    sgx_list_t                open_files;                       /// 打开的文件
    sgx_list_t                shared_memory;                    /// 共享内存链表

    sgx_uint_t                connection_n;                     /// 连接的个数
    sgx_uint_t                files_n;                          /// 打开文件的个数

    sgx_connection_t         *connections;                      /// 连接事件
    sgx_event_t              *read_events;                      /// 读取事件
    sgx_event_t              *write_events;                     /// 写入事件

    sgx_cycle_t              *old_cycle;

    sgx_str_t                 conf_file;                        /// 配置文件
    sgx_str_t                 conf_param;                       /// 配置参数
    sgx_str_t                 conf_prefix;                      /// 配置文件前缀
    sgx_str_t                 prefix;                           /// 前缀
    sgx_str_t                 lock_file;                        /// 锁文件
    sgx_str_t                 hostname;                         /// 主机名称
};


typedef struct {
    sgx_flag_t                daemon;
    sgx_flag_t                master;

    sgx_msec_t                timer_resolution;
    sgx_msec_t                shutdown_timeout;

    sgx_int_t                 worker_processes;
    sgx_int_t                 debug_points;

    sgx_int_t                 rlimit_nofile;
    off_t                     rlimit_core;

    int                       priority;

    sgx_uint_t                cpu_affinity_auto;
    sgx_uint_t                cpu_affinity_n;
    sgx_cpuset_t             *cpu_affinity;

    char                     *username;
    sgx_uid_t                 user;
    sgx_gid_t                 group;

    sgx_str_t                 working_directory;
    sgx_str_t                 lock_file;

    sgx_str_t                 pid;
    sgx_str_t                 oldpid;

    sgx_array_t               env;
    char                    **environment;

    sgx_uint_t                transparent;  /* unsigned  transparent:1; */
} sgx_core_conf_t;
