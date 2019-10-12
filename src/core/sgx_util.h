#ifndef UTIL_H
#define UTIL_H

#define SGX_MAX_LISTEN (1024)
#define SGX_BUFLEN (8192)
#define SGX_DELIM "="
#define SGX_CONF_OK 0
#define SGX_CONF_ERROR 100
#define SGX_MIN(a,b) ((a) < (b) ? (a) : (b))
#define SGX_MAX(a,b) ((a) > (b) ? (a) : (b))

typedef struct 
{
    void *root;
    int port;
    int thread_num;
}sgx_conf;

int read_con(char *filename, sgx_conf *cf, char *buf, int len);
// ssize_t Readline(int sgx_fd, void *sgx_ptr, size_t sgx_max_len);
// ssize_t Writeline(int sgx_fd, const void *sgx_ptr, size_t sgx_max_len);

#endif