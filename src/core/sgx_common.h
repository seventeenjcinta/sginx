#ifndef SGX_COMMON
#define SGX_COMMON

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define sgx_log_err(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define sgx_log_warn(M, ...) fprintf(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define sgx_log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define sgx_check(A, M, ...) if(!(A)) { sgx_log_err(M "\n", ##__VA_ARGS__); /* exit(1); */ }
#define sgx_check_exit(A, M, ...) if(!(A)) { log_err(M "\n", ##__VA_ARGS__); exit(1);}
#define check_debug(A, M, ...) if(!(A)) { debug(M "\n", ##__VA_ARGS__); /* exit(1); */}

#endif