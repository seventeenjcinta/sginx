#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define sh_isspace(c) \
    ((c==' ')||(c>='\t'&&c<='\r'))
    
//4.0 控制台打印错误信息, fmt必须是双引号括起来的宏
#define CERR(fmt, ...) \
    fprintf(stderr,"[%s:%s:%d][error %d:%s]" fmt "\r\n",\
         __FILE__, __func__, __LINE__, errno, strerror(errno),##__VA_ARGS__)

//4.1 控制台打印错误信息并退出, t同样fmt必须是 ""括起来的字符串常量
#define CERR_EXIT(fmt,...) \
    CERR(fmt,##__VA_ARGS__),exit(EXIT_FAILURE)

/// char 缓冲区大小
#define _INT_BUF (1024)
/// list 监听队列的大小
#define _INT_LIS (7)

/// 请求解析失败，客户端代码错误
extern inline void rsp_400(int cfd);
/// 请求文件未找到
extern inline void rsp_404(int cfd);
/// 不支持的请求
extern inline void rsp_501(int cfd);
/// 服务器内部错误，无法处理
extern inline void rsp_500(int cfd);
/// 请求成功
extern inline void rsp_200(int cfd);
/// 读取 fd 一行的内容，保存在 buf 中，返回读取内容的长度
int sgx_get_fd_line(int fd, char buf[], int size);
/// 将文件发送给客户端
void sgx_send_file(int cfd, const char *path);
/// 返回启动的服务器句柄，采用随机端口，如果传入 NULL 则不返回自动分配的端口
int sgx_start(uint16_t *pport);
/// 多线程处理的函数
void *sgx_request_accept(void *arg);
/// 处理客户端的 http 请求
void sgx_request_cgi(int cfd, const char *path, const char *type, const char *query);

int main(int argc, char* argv[])
{
    pthread_attr_t sgx_attr;
    uint16_t sgx_port;
    int sgx_fd;

    sgx_port = 0;
    sgx_fd = sgx_start(&sgx_port);
    printf("sgx running on port %u.\n", sgx_port);
    pthread_attr_init(&sgx_attr);
    pthread_attr_setdetachstate(&sgx_attr, PTHREAD_CREATE_DETACHED);    /// 可分离的
    while(1) {
        pthread_t sgx_tid;
        struct sockaddr_in sgx_caddr;
        socklen_t sgx_clen;
        int sgx_cfd;

        sgx_clen = sizeof(sgx_caddr);
        sgx_cfd = accept(sgx_fd, (struct sockaddr *)&sgx_caddr, &sgx_clen);
        if(sgx_cfd < 0) {
            CERR("accept sgx_fd = %d is error!", sgx_fd); 
            break;
        }
        if(pthread_create(&sgx_tid, &sgx_attr, sgx_request_accept, (void *)sgx_cfd) < 0) {
            CERR("pthread_create run is error!");
        }
    }
    pthread_attr_destroy(&sgx_attr);
    close(sgx_fd);

    return 0;
}

int sgx_get_fd_line(int sgx_fd, char sgx_buf[], int sgx_size)
{
    char *sgx_tail;
    char sgx_c;

    sgx_tail = sgx_buf;
    sgx_size --;
    while((sgx_tail - sgx_buf) < sgx_size) {
        if(read(sgx_fd, &sgx_c, 1) <= 0) {
            break;
        }
        if(sgx_c == '\r') {
            if(recv(sgx_fd, &sgx_c, 1, MSG_PEEK) > 0 && sgx_c == '\n') { ///  MSG_PEEK 表示查看数据,并不从系统缓冲区移走数据 
                read(sgx_fd, &sgx_c, 1);
            }
            else {
                *sgx_tail = '\n';
                *sgx_tail ++;
            }
            break;
        }
        *sgx_tail = sgx_c;
        *sgx_tail ++;
    }
    *sgx_tail = '\0';
    
    return sgx_tail - sgx_buf;
}

void sgx_send_file(int sgx_fd, const char *sgx_path)
{
    FILE *sgx_txt;
    char sgx_buf[_INT_BUF];

    while(sgx_get_fd_line(sgx_fd, sgx_buf, sizeof(sgx_buf)) > 0 && strcmp("\n", sgx_buf)) {
        /// void
    }
    if((sgx_txt = fopen(sgx_path, "r")) == NULL) {
        rsp_400(sgx_fd);
    }
    else {
        rsp_200(sgx_fd);
        while(!feof(sgx_txt) && fgets(sgx_buf, sizeof(sgx_buf), sgx_txt)) {
            write(sgx_fd, sgx_buf, strlen(sgx_buf));
        }
    }
    fclose(sgx_txt);
}

int sgx_start(uint16_t *sgx_port)
{
    int sgx_temp;
    int sgx_fd;
    struct sockaddr_in sgx_saddr = {AF_INET};

    // sgx_saddr ; /// IPV4
    sgx_temp = socket(PF_INET, SOCK_STREAM, 0);
    if(sgx_temp < 0) {
        /// BUG
        CERR_EXIT("sgx_temp");
    }
    sgx_fd = sgx_temp;
    sgx_saddr.sin_port = !sgx_port || !*sgx_port ? 0 : htons(*sgx_port);
    sgx_saddr.sin_addr.s_addr = INADDR_ANY;    /// 本地 IP
    /// 绑定端口
    sgx_temp = bind(sgx_fd, (struct sockaddr *)&sgx_saddr, sizeof(sgx_saddr));
    if(sgx_temp < 0) {
        CERR_EXIT("sgx_temp");
    }
    if(sgx_port && !*sgx_port) {
        socklen_t sgx_slen;

        sgx_slen = sizeof(sgx_saddr);
        sgx_temp = getsockname(sgx_fd, (struct sockaddr *)&sgx_saddr, &sgx_slen);
        if(sgx_temp < 0) {
            CERR_EXIT("sgx_temp");
        }
        *sgx_port = ntohs(sgx_saddr.sin_port);
    }
    sgx_temp = listen(sgx_fd, _INT_LIS);
    if(sgx_temp < 0) {
        CERR_EXIT("sgx_temp");
    }
    
    return sgx_fd;
}

/**
struct stat {
    dev_t st_dev; //device 文件的设备编号
    ino_t st_ino; //inode 文件的i-node
    mode_t st_mode; //protection 文件的类型和存取的权限
    nlink_t st_nlink; //number of hard links 连到该文件的硬连接数目, 刚建立的文件值为1.
    uid_t st_uid; //user ID of owner 文件所有者的用户识别码 
    gid_t st_gid; //group ID of owner 文件所有者的组识别码 
    dev_t st_rdev; //device type 若此文件为装置设备文件, 则为其设备编号 
    off_t st_size; //total size, in bytes 文件大小, 以字节计算 
    unsigned long st_blksize; //blocksize for filesystem I/O 文件系统的I/O 缓冲区大小. 
    u nsigned long st_blocks; //number of blocks allocated 占用文件区块的个数, 每一区块大小为512 个字节. 
    time_t st_atime; //time of lastaccess 文件最近一次被存取或被执行的时间, 一般只有在用mknod、 utime、read、write 与tructate 时改变.
    time_t st_mtime; //time of last modification 文件最后一次被修改的时间, 一般只有在用mknod、 utime 和write 时才会改变
    time_t st_ctime; //time of last change i-node 最近一次被更改的时间, 此参数会在文件所有者、组、 权限被更改时更新 
};
**/
void *sgx_request_accept(void *sgx_arg)
{
    char sgx_buf[_INT_BUF];
    char sgx_path[_INT_BUF >> 1];   /// 请求文件路径
    char sgx_type[_INT_BUF >> 5];   /// 请求类型，默认是 POST
    char *sgx_lt;
    char *sgx_rt;
    char *sgx_query;                /// url ？ 后的数据
    char *sgx_nb;
    struct stat sgx_st;
    int sgx_iscgi;
    int sgx_cfd;
    
    sgx_nb = sgx_buf;
    /// sgx_cfd = (int)sgx_arg;
    sgx_cfd = *(int *)sgx_arg;

    /// 请求错误
    if(sgx_get_fd_line(sgx_cfd, sgx_buf, sizeof(sgx_buf)) <= 0) {
        rsp_501(sgx_cfd);
        close(sgx_cfd);

        return NULL;
    }
    /// 合法请求
    sgx_lt = sgx_type;
    sgx_rt = sgx_nb;
    while(!sh_isspace(*sgx_rt) && (sgx_lt - sgx_type) < sizeof(sgx_type) - 1) {
        *sgx_lt = *sgx_rt;
        sgx_rt ++;
        sgx_lt ++;
    }
    *sgx_lt = '\0'; /// 已经将 buf 中开始不为 empty 部分塞入了 type 中
    if((sgx_iscgi = strcasecmp(sgx_type, "POST")) && strcasecmp(sgx_type, "GET")) { /// strcasecmp 忽略大小写比较字符串
        rsp_501(sgx_cfd);
        close(sgx_cfd);

        return NULL;
    }
    /// 去除空字符
    while(*sgx_rt && sh_isspace(*sgx_rt)) {
        sgx_rt ++;
    }
    /// 获取路径信息
    *sgx_path = '.';
    sgx_lt = sgx_path + 1;
    while(((sgx_lt - sgx_path) < sizeof(sgx_path) - 1) && !sh_isspace(*sgx_rt)) {
        *sgx_lt = *sgx_rt;
        sgx_rt ++;
        sgx_lt ++;
    }
    *sgx_lt = '\0';
    if(sgx_iscgi) {
        sgx_query = sgx_path;
        while(*sgx_query && *sgx_query != '?') {
            sgx_query ++;
        }
        if(*sgx_query == '?') {
            sgx_iscgi = 0;
            *sgx_query = '\0';
            sgx_query ++;
        }
    }
    if(stat(sgx_path, &sgx_st) < 0) {   /// 获取文件状态
        while(sgx_get_fd_line(sgx_cfd, sgx_buf, sizeof(sgx_buf)) > 0 && strcmp("\n", sgx_buf)) {

        }
        rsp_404(sgx_cfd);
        close(sgx_cfd);

        return NULL;
    }
    if((sgx_st.st_mode & S_IXUSR) || (sgx_st.st_mode & S_IXGRP) || (sgx_st.st_mode & S_IXOTH)) {
        sgx_iscgi = 0;
    }
    if(sgx_iscgi) {
        sgx_send_file(sgx_cfd, sgx_path);
    }
    else {
        sgx_request_cgi(sgx_cfd, sgx_path, sgx_type, sgx_query);
    }
    close(sgx_cfd);

    return NULL;
}


void sgx_request_cgi(int sgx_cfd, const char *sgx_path, const char *sgx_type, const char *sgx_query)
{
    char sgx_buf[_INT_BUF];
    int sgx_pocgi[2];   /// 0读取, 1写入
    int sgx_picgi[2];
    pid_t sgx_pid;
    int sgx_contlen;    /// 报文长度
    char sgx_c;

    sgx_contlen = -1;
    if(!strcasecmp(sgx_type, "POST")) {
        while(sgx_get_fd_line(sgx_cfd, sgx_buf, sizeof(sgx_buf)) > 0 && strcmp("\n", sgx_buf)) {
            sgx_buf[15] = '\0';
            if(!strcasecmp(sgx_buf, "Content-Length:")) {
                sgx_contlen = atoi(sgx_buf + 16);
            }
        }
        if(sgx_contlen == -1) {
            rsp_400(sgx_cfd);

            return ;
        }
    }
    else {
        while(sgx_get_fd_line(sgx_cfd, sgx_buf, sizeof(sgx_buf)) > 0 && strcmp("\n", sgx_buf)) {
            
        }
    }
    if(pipe(sgx_pocgi) < 0) {
        rsp_500(sgx_cfd);
        
        return ;
    }
    if(pipe(sgx_picgi) < 0) {
        close(sgx_pocgi[0]);
        close(sgx_pocgi[1]);
        rsp_500(sgx_cfd);

        return ;
    }
    sgx_pid = fork();
    if(sgx_pid < 0) {
        close(sgx_pocgi[0]);
        close(sgx_pocgi[1]);
        close(sgx_picgi[0]);
        close(sgx_picgi[1]);
        rsp_500(sgx_cfd);

        return ;
    }
    /// 多进程
    if(!sgx_pid) {
        dup2(sgx_pocgi[1], STDOUT_FILENO);
        dup2(sgx_picgi[0], STDIN_FILENO);
        close(sgx_pocgi[0]);
        close(sgx_picgi[1]); 
        sprintf(sgx_buf, "REQUEST_METHOD=%s", sgx_type);
        putenv(sgx_buf);
        if(!strcasecmp(sgx_buf, "POST")) {
            sprintf(sgx_buf, "CONTENT_LENGTH=%d", sgx_contlen);
        }
        else{
            sprintf(sgx_buf, "QUERY_STRING=%s", sgx_query);
        }
        putenv(sgx_buf);
        /// 成功，则调到新的执行体上
        execl(sgx_path, sgx_path, NULL);
        /// 防止 execl 失败子进程没有退出
        exit(EXIT_SUCCESS);
    }
    write(sgx_cfd, "HTTP1.0 200 OK\r\n", 17);
    close(sgx_pocgi[1]);
    close(sgx_picgi[0]);
    if(!strcasecmp(sgx_type, "POST")) {
        for(int i = 0; i < sgx_contlen; i ++) {
            read(sgx_cfd, &sgx_c, 1);
            write(sgx_picgi[1], &sgx_c, 1);
        }
    }
    /// 从子进程读取数据发送到客户端，多线程跨进程阻塞模型
    while(read(sgx_pocgi[0], &sgx_c, 1) > 0) {
        write(sgx_cfd, &sgx_c, 1);
    }
    close(sgx_pocgi[0]);
    close(sgx_picgi[1]);
    waitpid(sgx_pid, NULL, 0);
}