// #ifndef _SGX_CONFIG_H_INCLUDED_
// #define _SGX_CONFIG_H_INCLUDED_


// #include "sgx_auto_headers.h"


// #if defined __DragonFly__ && !defined __FreeBSD__
// #define __FreeBSD__        4
// #define __FreeBSD_version  480101
// #endif


// #if (SGX_FREEBSD)
// #include <sgx_freebsd_config.h>


// #elif (SGX_LINUX)
// #include <sgx_linux_config.h>


// #elif (SGX_SOLARIS)
// #include <sgx_solaris_config.h>


// #elif (SGX_DARWIN)
// #include <sgx_darwin_config.h>


// #elif (SGX_WIN32)
// #include <sgx_win32_config.h>


// #else /* POSIX */
// #include <sgx_posix_config.h>

// #endif


// #ifndef SGX_HAVE_SO_SNDLOWAT
// #define SGX_HAVE_SO_SNDLOWAT     1
// #endif


// #if !(SGX_WIN32)

// #define sgx_signal_helper(n)     SIG##n
// #define sgx_signal_value(n)      sgx_signal_helper(n)

// #define sgx_random               random

// /* TODO: #ifndef */
// #define SGX_SHUTDOWN_SIGNAL      QUIT
// #define SGX_TERMINATE_SIGNAL     TERM
// #define SGX_NOACCEPT_SIGNAL      WINCH
// #define SGX_RECONFIGURE_SIGNAL   HUP

// #if (SGX_LINUXTHREADS)
// #define SGX_REOPEN_SIGNAL        INFO
// #define SGX_CHANGEBIN_SIGNAL     XCPU
// #else
// #define SGX_REOPEN_SIGNAL        USR1
// #define SGX_CHANGEBIN_SIGNAL     USR2
// #endif

// #define sgx_cdecl
// #define sgx_libc_cdecl

// #endif

// typedef intptr_t        sgx_int_t;
// typedef uintptr_t       sgx_uint_t;
// typedef intptr_t        sgx_flag_t;


// #define SGX_INT32_LEN   (sizeof("-2147483648") - 1)
// #define SGX_INT64_LEN   (sizeof("-9223372036854775808") - 1)

// #if (SGX_PTR_SIZE == 4)
// #define SGX_INT_T_LEN   SGX_INT32_LEN
// #define SGX_MAX_INT_T_VALUE  2147483647

// #else
// #define SGX_INT_T_LEN   SGX_INT64_LEN
// #define SGX_MAX_INT_T_VALUE  9223372036854775807
// #endif


// #ifndef SGX_ALIGNMENT
// #define SGX_ALIGNMENT   sizeof(unsigned long)    /* platform word */
// #endif

// #define sgx_align(d, a)     (((d) + (a - 1)) & ~(a - 1))
// #define sgx_align_ptr(p, a)                                                   \
//     (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))


// #define sgx_abort       abort


// /* TODO: platform specific: array[SGX_INVALID_ARRAY_INDEX] must cause SIGSEGV */
// #define SGX_INVALID_ARRAY_INDEX 0x80000000


// /* TODO: auto_conf: sgx_inline   inline __inline __inline__ */
// #ifndef sgx_inline
// #define sgx_inline      inline
// #endif

// #ifndef INADDR_NONE  /* Solaris */
// #define INADDR_NONE  ((unsigned int) -1)
// #endif

// #ifdef MAXHOSTNAMELEN
// #define SGX_MAXHOSTNAMELEN  MAXHOSTNAMELEN
// #else
// #define SGX_MAXHOSTNAMELEN  256
// #endif


// #define SGX_MAX_UINT32_VALUE  (uint32_t) 0xffffffff
// #define SGX_MAX_INT32_VALUE   (uint32_t) 0x7fffffff


// #if (SGX_COMPAT)

// #define SGX_COMPAT_BEGIN(slots)  uint64_t spare[slots];
// #define SGX_COMPAT_END

// #else

// #define SGX_COMPAT_BEGIN(slots)
// #define SGX_COMPAT_END

// #endif


// #endif /* _SGX_CONFIG_H_INCLUDED_ */
