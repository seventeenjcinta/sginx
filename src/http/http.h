#ifndef HTTP_H
#define HTTP_H

#include "sgx_list.h"
#include "sgx_util.h"
#include "http_request.h"

#define SGX_MAX_LINE 8192
#define SGX_SHORT_LINE 512

#define sgx_str3_cmp(m, c0, c1, c2, c3)                                       \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)
#define sgx_str3Ocmp(m, c0, c1, c2, c3)                                       \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)

#define sgx_str4cmp(m, c0, c1, c2, c3)                                        \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)

/// Multipurpose Internet Mail Extensions, “多用途互联网邮件扩展”，是当前广泛应用的一种电子邮件技术规范
typedef struct 
{
    const char *type;
    const char *value;
}mime_type;

void sgx_request_run(void *fd);


#endif