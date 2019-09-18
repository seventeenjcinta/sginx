typedef struct 
{
    sgx_file_t file;    /// 文件的属性
    sgx_buf_t *buffer;  /// 文件的内容
    sgx_buf_t *dump;
    sgx_uint_ line;     /// 行数
}sgx_conf_file_t;


struct sgx_conf_s
{
    char *name;                     /// 存放当前解析到的指令
    sgx_array_t *args;              /// 存放该指令包含的参数
    sgx_cycle_t *cycle;
    sgx_pool_t *pool;
    sgx_pool_t *temp_pool;
    sgx_conf_file_t *conf_file;     /// 存放 Nginx 配置文件的相关信息
    sgx_log_t *log;
    void *ctx;
    sgx_uint_t module_type;
    sgx_uint_t cmd_type;
    sgx_conf_handler_pt handler;
    void *handler_conf;
};
