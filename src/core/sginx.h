#include <unistd.h>

#define SGX_MAX_LISTEN (1024)

ssize_t Readline(int sgx_fd, void *sgx_ptr, size_t sgx_max_len);
ssize_t Writeline(int sgx_fd, const void *sgx_ptr, size_t sgx_max_len);