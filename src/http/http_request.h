#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <time.h>
#include "http.h"

#define SGX_AGAIN    EAGAIN
#define SGX_HTTP_PARSE_INVALID_METHOD        10
#define SGX_HTTP_PARSE_INVALID_REQUEST       11
#define SGX_HTTP_PARSE_INVALID_HEADER        12
#define SGX_HTTP_UNKNOWN                     0x0001
#define SGX_HTTP_GET                         0x0002
#define SGX_HTTP_HEAD                        0x0004
#define SGX_HTTP_POST                        0x0008
#define SGX_HTTP_OK                          200
#define SGX_HTTP_NOT_MODIFIED                304
#define SGX_HTTP_NOT_FOUND                   404
#define SGX_MAX_BUF                          8124
#define SGX_TIME_EPS                         1e-6

typedef struct
{
    void *root;
    int fd;
    int epfd;
    char buf[SGX_MAX_BUF];              /// ring buffer
    size_t pos;
    size_t last;
    int state;
    void *request_start;
    void *method_end;                   /// [)
    int method;
    void *uri_start;
    void *uri_end;                      /// not include uri_end*
    void *path_start;
    void *path_end;
    void *query_start;
    void *query_end;
    int http_major;
    int http_minor;
    void *request_end;
    struct sgx_list_head list;          /// 放 http header
    void *cur_header_key_start;
    void *cur_header_key_end;
    void *cur_header_value_start;
    void *cur_header_value_end;
    void *timer;
} sgx_http_request;

typedef struct {
    int fd;
    int keep_alive;
    time_t mtime;       /// 文件的修改时间
    int modified;       /// 将 If-modified-since 字段与 mtime 进行比较，以确定自上次以来是否已修改文件
    int status;
} sgx_http_out;

typedef struct 
{
    void *key_start;
    void *key_end;          /// [)
    void *value_start;
    void *value_end;
    sgx_list_head list;
}sgx_http_header;

typedef int (*sgx_http_header_handler_pt)(sgx_http_request *r, sgx_http_out *o, char *data, int len);

typedef struct 
{
    char *name;
    sgx_http_header_handler_pt handler;
}sgx_http_header_handle;

void sgx_http_handle_header(sgx_http_request *r, sgx_http_out *o);
int sgx_http_close_conn(sgx_http_request *r);
int sgx_init_request(sgx_http_request *request, int fd, int epfd, sgx_conf *config);
int sgx_free_request(sgx_http_request *r);
int sgx_init_out(sgx_http_out *o, int fd);
int sgx_free_out(sgx_http_out *o);
const char *get_shortmsg_from_status_code(int status_code);
extern sgx_http_header_handle     sgx_http_headers_in[];

#endif