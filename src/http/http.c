#include <strings.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "http.h"
#include "http_request.h"
#include "sgx_epoll.h"
#include "sgx_core.h"
#include "event_timer.h"

static char *ROOT = NULL;

mime_type sgx_mime[] = 
{
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/msword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css"},
    {NULL ,"text/plain"}
};

void sgx_request_run(void *ptr) {
    sgx_http_request *request;
    int fd;
    int result;
    int n;
    char *plast;
    char filename[SGX_SHORT_LINE];
    size_t remain_size;

    request = (sgx_http_request *)ptr;
    fd = request -> fd;
    ROOT = request -> root;
    sgx_timer_del(request);
    while(1) {
        plast = &request -> buf[request -> last % MAX_BUF];
        remain_size = SGX_MIN(SGX_MAX_BUF - (request -> last - request -> pos) - 1, SGX_MAX_BUF - request -> last % SGX_MAX_BUF);
        n = read(fd, plast, remain_size);
        if(request -> last - request -> pos >= SGX_MAX_BUF) {
            sgx_log_err("request buffer overflow!");
        }
        if(!n) {
            sgx_log_info("read return 0, ready to close fd %d, remain_size = %zu", fd, remain_size);
            goto err;
        }
        if(n < 0) {
            if(errno != EAGAIN) {
                sgx_log_err("read err errno = %d", errno);\
                goto err;
            }
            break;
        }
        request -> last += n;
        if(request -> last - request -> pos >= SGX_MAX_BUF) {
            sgx_log_err("request buffer overflow!");
        }
    }
}
