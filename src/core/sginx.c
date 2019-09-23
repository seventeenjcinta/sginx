#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sginx.h"
#define SGX_ECHO_PORT (2002)
#define SGX_MAX_BUF (1001)

int main(int argc, char *argv[])
{
    struct sockaddr_in sgx_server_sockaddr;
    int sgx_listen_socket;
    int sgx_port;
    int sgx_temp;
    char *sgx_endptr;

    if(argc == 2) {
        sgx_port = strtol(argv[1], &sgx_endptr, 0);
        if(*sgx_endptr) {
            fprintf(stderr, "ECHOSERV: Invalid port number.\n");    
            exit(EXIT_FAILURE);
        }
    }
    else if(argc < 2) {
        sgx_port = SGX_ECHO_PORT;
    }
    else {
        fprintf(stderr, "ECHOSERV: Invalid arguments.\n");
	    exit(EXIT_FAILURE);
    }
    sgx_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(sgx_listen_socket < 0) {
        fprintf(stderr, "ECHOSERV: Error creating listening socket.\n");
        exit(EXIT_FAILURE);
    }
    memset(&sgx_server_sockaddr, 0, sizeof(sgx_server_sockaddr));
    sgx_server_sockaddr.sin_family = AF_INET;
    sgx_server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY); /// 本地环境
    sgx_server_sockaddr.sin_port = htons(sgx_port);
    sgx_temp = bind(sgx_listen_socket, (struct sockaddr *)&sgx_server_sockaddr, sizeof(sgx_server_sockaddr));
    if(sgx_temp < 0) {
        fprintf(stderr, "ECHOSERV: Error calling bind()\n");
        exit(EXIT_FAILURE);
    }
    sgx_temp = listen(sgx_listen_socket, SGX_MAX_LISTEN);
    if(sgx_temp < 0) {
        fprintf(stderr, "ECHOSERV: Error calling listen()\n");
        exit(EXIT_FAILURE);
    }
    while(1) {
        int sgx_connect_socket;
        char sgx_buffer[SGX_MAX_BUF];

        printf("seventeen is good.\n");
        sgx_connect_socket = accept(sgx_listen_socket, NULL, NULL);
        if(sgx_connect_socket < 0) {
            fprintf(stderr, "ECHOSERV: Error calling accept()\n");
            exit(EXIT_FAILURE);
        }
        printf("accept succeed.\n");
        Readline(sgx_connect_socket, sgx_buffer, SGX_MAX_BUF - 1);
        printf("read is %s\n", sgx_buffer);
        Writeline(sgx_connect_socket, sgx_buffer, strlen(sgx_buffer));
        if(close(sgx_connect_socket) < 0) {
            fprintf(stderr, "ECHOSERV: Error calling close()\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

ssize_t Readline(int sgx_fd, void *sgx_ptr, size_t sgx_max_len)
{
    ssize_t sgx_temp;
    ssize_t sgx_len;
    char sgx_c;
    char *sgx_buffer;

    sgx_len = 1;
    sgx_buffer = sgx_ptr;
    while(sgx_len < sgx_max_len) {
        sgx_temp = read(sgx_fd, &sgx_c, 1);
        if(sgx_temp == 1) {
            *sgx_buffer = sgx_c;
            sgx_buffer ++;
            if(sgx_c == '\n') {
                break;
            }
        }
        else if(!sgx_temp) {
            if(sgx_len == 1) {
                return 0;
            }
            else {
                break;
            }
        }
        else {
            if(errno == EINTR) {
                continue;
            }

            return -1;
        }
        sgx_len ++;
    }
    *sgx_buffer = 0;
    
    return sgx_len;
}

ssize_t Writeline(int sgx_fd, const void *sgx_ptr, size_t sgx_max_len)
{
    size_t sgx_left;
    const char *sgx_buffer;

    sgx_buffer = sgx_ptr;
    sgx_left = sgx_max_len;
    while(sgx_left > 0) {
        ssize_t sgx_num;

        sgx_num = write(sgx_fd, sgx_buffer, sgx_left);
        if(sgx_num <= 0) {
            if(errno == EINTR) {
                sgx_num = 0;
            }
            else {
                return -1;
            }
        }

        sgx_left -= sgx_num;
        sgx_buffer += sgx_num;
    }

    return sgx_max_len;
}

// #include <sgx_config.h>
// #include <sgx_core.h>
// #include <sginx.h>

// /// 平滑重启   inherited 遗传
// static sgx_int_t sgx_add_inherited_sockets(sgx_cycle_t *cycle);
// /// 解析命令行中的参数
// static sgx_int_t sgx_get_options(int argc, char *const *argv);
// /// 将 sgx_get_options 获取的参数取值赋值到 cycle 中
// static sgx_int_t sgx_provess_options(shx_cycle_t *cycle); 
// /// 保存命令行中的参数和变量，放到全局变量 sgx_argv 中
// static sgx_int_t sgx_save_argv(sgx_cycle_t *cycle, int argc, char *const *argv);

// int sgx_cdecl int main(int argc, char const *argv[])
// {
    
//     /// 解析外部参数
//     if(sgx_get_options(argc, argv) != SGX_OK) {
//         return 1;
//     }

//     return 0;
// }

// /// ./sginx -s stop|start|restart
// static sgx_int_t sgx_get_options(int argc, char *const *argv)
// {
//     for(sgx_int_t i = 1; i < argc; i ++) {
//         u_char *p;

//         p = (u_char *)argv[i];
//             if(*p ++ != '-') {
//             sgx_log_stderr(0, "invalid option: \"%s\"", argv[i]);

//             return SGX_ERROR;
//         }
//         while(*p) {
//             switch (*p ++) {
//             case '?':
//             case 'h':
//                 sgx_show_version = 1;
//                 sgx_show_help = 1;
//                 break;

//             case 'v':
//                 sgx_show_version = 1;
//                 break;

//             case 'V':
//                 sgx_show_version = 1;
//                 sgx_show_configure = 1;
//                 break;

//             case 't':
//                 sgx_test_config = 1;
//                 break;

//             case 'T':
//                 sgx_test_config = 1;
//                 sgx_dump_config = 1;
//                 break;

//             case 'q':
//                 sgx_quiet_mode = 1;
//                 break;

//             case 'p':
//                 if (*p) {
//                     sgx_prefix = p;
//                     goto next;
//                 }

//                 if (argv[++i]) {
//                     sgx_prefix = (u_char *) argv[i];
//                     goto next;
//                 }

//                 sgx_log_stderr(0, "option \"-p\" requires directory name");
//                 return SGX_ERROR;

//             case 'c':
//                 if (*p) {
//                     sgx_conf_file = p;
//                     goto next;
//                 }

//                 if (argv[++i]) {
//                     sgx_conf_file = (u_char *) argv[i];
//                     goto next;
//                 }

//                 sgx_log_stderr(0, "option \"-c\" requires file name");
//                 return SGX_ERROR;

//             case 'g':
//                 if (*p) {
//                     sgx_conf_params = p;
//                     goto next;
//                 }

//                 if (argv[++i]) {
//                     sgx_conf_params = (u_char *) argv[i];
//                     goto next;
//                 }

//                 sgx_log_stderr(0, "option \"-g\" requires parameter");
//                 return SGX_ERROR;

//             case 's':
//                 if (*p) {
//                     sgx_signal = (char *) p;

//                 } else if (argv[++i]) {
//                     sgx_signal = argv[i];

//                 } else {
//                     sgx_log_stderr(0, "option \"-s\" requires parameter");
//                     return SGX_ERROR;
//                 }

//                 if (sgx_strcmp(sgx_signal, "stop") == 0
//                     || sgx_strcmp(sgx_signal, "quit") == 0
//                     || sgx_strcmp(sgx_signal, "reopen") == 0
//                     || sgx_strcmp(sgx_signal, "reload") == 0)
//                 {
//                     sgx_process = SGX_PROCESS_SIGNALLER;
//                     goto next;
//                 }

//                 sgx_log_stderr(0, "invalid option: \"-s %s\"", sgx_signal);
//                 return SGX_ERROR;

//             default:
//                 sgx_log_stderr(0, "invalid option: \"%c\"", *(p - 1));
//                 return SGX_ERROR;
//             }
//         }
//     next:
//         continue;
//     }

//     return SGX_OK;
// }

// /// 通过环境变量 SGINX 完成 socket 的继承，继承来的 socket 会被放到 init_cycle 的 listening 数组中
// static sgx_int_t sgx_add_inherited_sockets(sgx_cycle_t *cycle)
// {
//     u_char *inherited;
//     u_char *p;
//     u_char *v;
//     sgx_int_t t;

//     /// 获取宏环境变量 SGINX 的值，每个 socket 由冒号或者分号隔开
//     inherited = (u_char *)getenv(SGINX_VAR);
//     if(inherited == NULL) {
//         return SGX_OK;
//     }
//     sgx_log_error(SGX_LOG_NOTIC, cycle -> log, 0, "using inherited sockets form \"%s\"", inherited);
//     /// 初始化 sgx_cycle.listening 数组
//     if(sgx_array_init(&cycle -> listening, cycle -> pool, 10, sizeof(sgx_listening_t)) != SGX_OK) {
//         return SGX_ERROR;
//     }
//     p = inhertied;
//     v = p;
//     while(*p) {
//         sgx_listening_t *listening;

//         if(*p == ':' || *p == ";") {
//             t = sgx_atoi(v, p - v);
//             if(t == SGX_ERROR) {
//                 sgx_log_error(SGX_LOG_EMERG, cycle -> log, 0, "invalid socket number \"%s\" in " NGINX_VAR " environment variable, ignoring the rest"" of the variable", v);
//                 break;
//             }
//             v = p + 1;
//             listening = sgx_array_push(&cycle -> listening);
//             if(listening == NULL) {
//                 return SGX_ERROR;
//             }
//             sgx_memzero(listening, sizeof(sgx_listening_t));
//             listening -> fd = (sgx_socket_t) t;
//         }
//         p ++;
//     }
//     if(v != p) {
//         sgx_log_error(SGX_LOG_EMERG, cycle -> log, 0, "invalid socket number \"%s\" in " NGINX_VAR" environment variable, ignoring", v);
//     }
//     sgx_inherited = 1;

//     return sgx_set_inherited_sockets(cycle);
// }