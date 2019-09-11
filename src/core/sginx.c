#include <sgx_config.h>
#include <sgx_core.h>
#include <sginx.h>

/// 平滑重启   inherited 遗传
static sgx_int_t sgx_add_inherited_sockets(sgx_cycle_t *cycle);
/// 解析命令行中的参数
static sgx_int_t sgx_get_options(int argc, char *const *argv);
/// 将 sgx_get_options 获取的参数取值赋值到 cycle 中
static sgx_int_t sgx_provess_options(shx_cycle_t *cycle); 
/// 保存命令行中的参数和变量，放到全局变量 sgx_argv 中
static sgx_int_t sgx_save_argv(sgx_cycle_t *cycle, int argc, char *const *argv);

int sgx_cdecl int main(int argc, char const *argv[])
{
    
    /// 解析外部参数
    if(sgx_get_options(argc, argv) != SGX_OK) {
        return 1;
    }

    return 0;
}

/// ./sginx -s stop|start|restart
static sgx_int_t sgx_get_options(int argc, char *const *argv)
{
    for(sgx_int_t i = 1; i < argc; i ++) {
        u_char *p;

        p = (u_char *)argv[i];
            if(*p ++ != '-') {
            sgx_log_stderr(0, "invalid option: \"%s\"", argv[i]);

            return SGX_ERROR;
        }
        while(*p) {
            switch (*p ++) {
            case '?':
            case 'h':
                sgx_show_version = 1;
                sgx_show_help = 1;
                break;

            case 'v':
                sgx_show_version = 1;
                break;

            case 'V':
                sgx_show_version = 1;
                sgx_show_configure = 1;
                break;

            case 't':
                sgx_test_config = 1;
                break;

            case 'T':
                sgx_test_config = 1;
                sgx_dump_config = 1;
                break;

            case 'q':
                sgx_quiet_mode = 1;
                break;

            case 'p':
                if (*p) {
                    sgx_prefix = p;
                    goto next;
                }

                if (argv[++i]) {
                    sgx_prefix = (u_char *) argv[i];
                    goto next;
                }

                sgx_log_stderr(0, "option \"-p\" requires directory name");
                return SGX_ERROR;

            case 'c':
                if (*p) {
                    sgx_conf_file = p;
                    goto next;
                }

                if (argv[++i]) {
                    sgx_conf_file = (u_char *) argv[i];
                    goto next;
                }

                sgx_log_stderr(0, "option \"-c\" requires file name");
                return SGX_ERROR;

            case 'g':
                if (*p) {
                    sgx_conf_params = p;
                    goto next;
                }

                if (argv[++i]) {
                    sgx_conf_params = (u_char *) argv[i];
                    goto next;
                }

                sgx_log_stderr(0, "option \"-g\" requires parameter");
                return SGX_ERROR;

            case 's':
                if (*p) {
                    sgx_signal = (char *) p;

                } else if (argv[++i]) {
                    sgx_signal = argv[i];

                } else {
                    sgx_log_stderr(0, "option \"-s\" requires parameter");
                    return SGX_ERROR;
                }

                if (sgx_strcmp(sgx_signal, "stop") == 0
                    || sgx_strcmp(sgx_signal, "quit") == 0
                    || sgx_strcmp(sgx_signal, "reopen") == 0
                    || sgx_strcmp(sgx_signal, "reload") == 0)
                {
                    sgx_process = SGX_PROCESS_SIGNALLER;
                    goto next;
                }

                sgx_log_stderr(0, "invalid option: \"-s %s\"", sgx_signal);
                return SGX_ERROR;

            default:
                sgx_log_stderr(0, "invalid option: \"%c\"", *(p - 1));
                return SGX_ERROR;
            }
        }
    next:
        continue;
    }

    return SGX_OK;
}

/// 通过环境变量 SGINX 完成 socket 的继承，继承来的 socket 会被放到 init_cycle 的 listening 数组中
static sgx_int_t sgx_add_inherited_sockets(sgx_cycle_t *cycle)
{
    u_char *inherited;
    u_char *p;
    u_char *v;
    sgx_int_t t;

    /// 获取宏环境变量 SGINX 的值，每个 socket 由冒号或者分号隔开
    inherited = (u_char *)getenv(SGINX_VAR);
    if(inherited == NULL) {
        return SGX_OK;
    }
    sgx_log_error(SGX_LOG_NOTIC, cycle -> log, 0, "using inherited sockets form \"%s\"", inherited);
    /// 初始化 sgx_cycle.listening 数组
    if(sgx_array_init(&cycle -> listening, cycle -> pool, 10, sizeof(sgx_listening_t)) != SGX_OK) {
        return SGX_ERROR;
    }
    p = inhertied;
    v = p;
    while(*p) {
        sgx_listening_t *listening;

        if(*p == ':' || *p == ";") {
            t = sgx_atoi(v, p - v);
            if(t == SGX_ERROR) {
                sgx_log_error(SGX_LOG_EMERG, cycle -> log, 0, "invalid socket number \"%s\" in " NGINX_VAR " environment variable, ignoring the rest"" of the variable", v);
                break;
            }
            v = p + 1;
            listening = sgx_array_push(&cycle -> listening);
            if(listening == NULL) {
                return SGX_ERROR;
            }
            sgx_memzero(listening, sizeof(sgx_listening_t));
            listening -> fd = (sgx_socket_t) t;
        }
        p ++;
    }
    if(v != p) {
        sgx_log_error(SGX_LOG_EMERG, cycle -> log, 0, "invalid socket number \"%s\" in " NGINX_VAR" environment variable, ignoring", v);
    }
    sgx_inherited = 1;

    return sgx_set_inherited_sockets(cycle);
}