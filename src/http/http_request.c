#ifndef _GNU_SOURCE
/// http://stackoverflow.com/questions/15334558/compiler-gets-warnings-when-using-strptime-function-ci */
#define _GNU_SOUTCE
#endif

#include <math.h>
#include <time.h>
#include <unistd.h>
#include "sgx_core.h"
#include "http.h"
#include "http_request.h"

static int sgx_http_process_ignore(sgx_http_request *request, sgx_http_out *out, char *data, int len);
static int sgx_http_process_connection(sgx_http_request *request, sgx_http_out *out, char *data, int len);
static int sgx_http_process_if_modified_since(sgx_http_request *request, sgx_http_out *out, char *data, int len);

sgx_http_header_handle sgx_http_headers_in[] = {
    {"Host", sgx_http_process_ignore},
    {"Connection", sgx_http_process_connection},
    {"If-Modified-Since", sgx_http_process_if_modified_since},
    {"", sgx_http_process_ignore}
};  

int sgx_init_request(sgx_http_request *request, int fd, int epfd, sgx_conf *config)
{   
    request -> fd = fd;
    request -> epfd = epfd;
    request -> last = 0;
    request -> pos = request -> last;
    request -> state = 0;
    request -> root = config -> root;
    INIT_LIST_HEAD(&(request -> list));

    return SGX_OK;
}

int sgx_free_request(sgx_http_request *request)
{
    (void) request;

    return SGX_OK;
}

int sgx_init_out(sgx_http_out *out, int fd)
{
    out -> fd = fd;
    out -> keep_alive = 0;
    out -> modified = 1;
    out -> status = 0;

    return SGX_OK;
}

int sgx_free_out(sgx_http_out *out)
{
    (void) out;

    return SGX_OK;
}

void sgx_http_handle_header(sgx_http_request *request, sgx_http_out *out)
{
    sgx_list_head *pos;

    list_for_each(pos, &(request -> list)) {
        sgx_http_header *head;
        
        head = list_entry(pos, sgx_http_header, list);
        for(sgx_http_header_handle *i = sgx_http_headers_in; strlen(i -> name) > 0; i ++) {
            if(strncmp(head -> key_start, i -> name, head -> key_end - head -> key_start) == 0) {
                int len;

                len = head -> value_end - head -> value_start;
                (*(i -> handler))(request, out, head -> value_start, len);
                break;
            }
        }
        list_del(pos);
        free(head);
    }
}

int sgx_http_close_conn(sgx_http_request *request)
{
    /// 关闭文件描述符会自动将其从所有epoll集中删除
    /// https://stackoverflow.com/questions/8707601/is-it-necessary-to-deregister-a-socket-from-epoll-before-closing-it
    close(request -> fd);
    free(request);

    return SGX_OK;
}

static int sgx_http_process_ignore(sgx_http_request *request, sgx_http_out *out, char *data, int len)
{
    (void) request;
    (void) out;
    (void) data;
    (void) len;

    return SGX_OK;
}
static int sgx_http_process_connection(sgx_http_request *request, sgx_http_out *out, char *data, int len)
{
    (void) request;
    if(strncasecmp("keep-alive", data, len) == 0) {
        out -> keep_alive = 1;
    }

    return SGX_OK;
}

static int sgx_http_process_if_modified_since(sgx_http_request *request, sgx_http_out *out, char *data, int len)
{
    struct tm tm;
    time_t client_time;
    double time_diff;

    (void) request;
    (void) len;
    if(strptime(data, "%a, %d %b %Y %H:%M:%S GMT", &tm) == (char *)NULL) {
        return SGX_OK;
    }
    client_time = mktime(&tm);
    time_diff = difftime(out -> mtime, client_time);
    if(fabs(time_diff) < SGX_TIME_EPS) {
        out -> modified = 0;
        out -> status - SGX_HTTP_NOT_MODIFIED;
    }

    return SGX_OK;
}

const char *get_shortmsg_from_status_code(int status_code)
{
    /// 参考了：http://users.polytech.unice.fr/~buffa/cours/internet/POLYS/servlets/Servlet-Tutorial-Response-Status-Line.html
    if(status_code == SGX_HTTP_OK) {
        return "OK";
    }
    if(status_code == SGX_HTTP_NOT_MODIFIED) {
        return "Not Modified";
    }
    if(status_code == SGX_HTTP_NOT_FOUND) {
        return "Not Found";
    }

    return "Unknown";
}