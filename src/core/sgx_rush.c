#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "sgx_rush.h"
#include "event_timer.h"
#include "sgx_log.h"
#include "sgx_epoll.h"
#include "sgx_common.h"
#include "http/http.h"
#define SGX_ECHO_PORT (2002)
#define SGX_MAX_BUF (1001)

extern struct epoll_event *events;

int main(int argc, char *argv[])
{
    int result;
    /// 配置
    char *conf_file;
    char conf_buf[SGX_BUFLEN];
    sgx_conf config;
    sgx_http_request *event_request;
    /// 信号量
    struct sigaction sig;
    /// socket    
    struct sockaddr_in client_addr;
    socklen_t inlen;
    int listen_socket;
    int port;
    char *endptr;
    /// epoll
    struct epoll_event event;
    int epoll_fd;

    /// 读取配置文件
    conf_file = BASE_CONF;
    result = read_conf(conf_file, &config, conf_buf, SGX_BUFLEN);
    if(result != SGX_CONF_OK) {
        sgx_log_err("read conf err");
    }
    /// 为 SIGPIPE 安装信号句柄，当通过远程关闭 fd 时，对该 fd 进行写入将导致系统发送 SIGPIPE 到此过程，退出程序
    memset(&sig, '\0', sizeof(sig));
    sig.sa_handler = SIG_IGN;
    sig.sa_flags = 0;
    if(sigaction(SIGPIPE, &sig, NULL)) {
        sgx_log_err("install sigal handler for SIGPIPE");
        exit(EXIT_FAILURE);
    }
    /// 初始化 listening socket
    /// 初始化 client_addr 和 inlen  要不然会有 accept Invalid argument 报错
    inlen = 1;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    listen_socket = sgx_get_listen_fd(port);
    if(listen_socket < 0) {
        sgx_log_err("call get_listen_fd()");
    } 
    /// 使套接字无阻塞。 如果侦听套接字是阻塞套接字，则从epoll出来并接受最后一个连接后，下一个accpet将阻塞
    result = sgx_socket_unblock(listen_socket);
    if(result < 0) {
        sgx_log_err("make_socket_un_blocking");
    }
    epoll_fd = sgx_epoll_create(0);
    /// epoll 和 http_request
    event_request = (sgx_http_request *)malloc(sizeof(sgx_http_request));
    if(event_request == NULL) {
        sgx_log_err("malloc(sizeof(sgx_http_request)");
    }
    sgx_init_request(event_request, listen_socket, epoll_fd, &config);
    event.data.ptr = (void *)event_request;
    event.events = EPOLLIN | EPOLLET;
    sgx_epoll_add(epoll_fd, listen_socket, &event);
    /// 初始化 timer
    sgx_timer_init();
    sgx_log_info("super goose X rush!");
    while(1) {
        int n;
        int time;

        /// 等待时间
        time = sgx_timer_find();
        n = sgx_epool_wait(epoll_fd, events, SGX_MAX_EVENTS, time);
        sgx_handle_expire_timers();

        for(int i = 0; i < n; i ++) {
            sgx_http_request *request; 
            int fd;

            request = (sgx_http_request *)events[i].data.ptr;
            fd = request -> fd;            
            if(listen_socket == fd) {
                /// 原始套接字，建立连接
                while(1) {
                    int accept_fd;
                    sgx_http_request *new_request;
                    
                    /// 建立连接
                    accept_fd = accept(listen_socket, (struct sockaddr *)&client_addr, &inlen);
                    if(accept_fd < 0) {
                        if((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            /// 所有的进程都在连接
                            break;
                        }
                        else {
                            sgx_log_err("accept");
                            break;
                        }
                    }
                    result = sgx_socket_unblock(accept_fd);
                    if(result != 0) {
                        sgx_log_err("make_socket_un_blocking");
                    }
                    sgx_log_info("new connection fd: %d", accept_fd);
                    new_request = (sgx_http_request *)malloc(sizeof(sgx_http_request));
                    if(new_request == NULL) {
                        sgx_log_err("malloc(sizeof(sgx_http_request)");
                    }
                    event.data.ptr = (void *)new_request;
                    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                    sgx_epoll_add(new_request, accept_fd, &event);
                    sgx_timer_add(request, SGX_TIMEOUT_DEFAULT, sgx_http_close_conn);
                }
            }
            else {
                if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))) {
                    sgx_log_err("epoll fd: %d", request -> fd);
                    close(fd);
                    continue;
                }
                sgx_log_info("new data from fd %d", fd);
                sgx_init_request
            }
        }
    }

    return 0;
}

int sgx_get_listen_fd(int port)
{
    struct sockaddr_in server_sockaddr;
    int listen_fd;
    int optval;
    int result;
    
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd < 0) {
        sgx_log_err("creating listening socket");
        
        return -1;
    }
    optval = 1;
    result = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
    if(result < 0) {
        sgx_log_err("set socket opt");

        return -1;
    }
    memset(&server_sockaddr, 0, sizeof(server_sockaddr));
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY); /// 本地环境
    server_sockaddr.sin_port = htons((unsigned short)port);
    result = bind(listen_fd, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr));
    if(result < 0) {
        sgx_log_err("calling bind()");
        
        return -1;
    }
    result = listen(listen_fd, SGX_MAX_LISTEN);
    if(result < 0) {
        sgx_log_err("calling listen()");
        
        return -1;
    }

    return listen_fd;
}

int sgx_socket_unblock(int fd)
{
    int flag;

    flag = fcntl(fd, F_SETFL, 0);
    if(flag == -1) {
        sgx_log_err("call fcntl()");
        
        return -1;
    }
    flag |= O_NONBLOCK;
    flag = fcntl(fd, F_SETFL, flag);
    if(flag == -1) {
        sgx_log_err("call fcntl()");
        
        return -1;
    }

    return 0;
}

ssize_t Readline(int sgx_fd, void *sgx_ptr, size_t sgx_max_len)
{
    ssize_t result;
    ssize_t sgx_len;
    char sgx_c;
    char *sgx_buffer;

    sgx_len = 1;
    sgx_buffer = sgx_ptr;
    while(sgx_len < sgx_max_len) {
        result = read(sgx_fd, &sgx_c, 1);
        if(result == 1) {
            *sgx_buffer = sgx_c;
            sgx_buffer ++;
            if(sgx_c == '\n') {
                break;
            }
        }
        else if(!result) {
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