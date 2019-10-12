#include <sys/socket.h>
#include <sys/types.h>
#include "sgx_core.h"

struct sgx_listening
{
    int socket_fd;                      /// socket 句柄
    struct sockaddr *socket_addr;
    socklen_t socket_len;
    size_t addr_text_max_len;
    u_char *data;                       /// 存储 IP 地址的字符串
    size_t data_len;
    int socket_type;                    /// 套接字类型
    int backlog;                        /// 表示允许正在通过三次握手建立 TCP 连接但是还没有任何进程开始处理的连接最大个数
    int receive_buf;                    /// 对于套接字的接收缓冲区大小
    int send_buf;                       /// 对于套接字的发送缓冲区大小
    sgx_connection_handler_pt hadler;   /// 新的 HTTP 连接建立成功后的处理方法
    void *server;
    struct sgx_listening *pre;          
    struct sgx_connection  *connection;

};

struct sgx_connection
{

};