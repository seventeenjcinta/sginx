// #include <sys/type.h>
#include "http.h"
#include "http_parse.h"
#include "sgx_code.h"
#include "sgx_log.h"

int sgx_http_parse_request_line(sgx_http_request *request) 
{
    size_t pos;

    enum {
        sw_spaces_before_uri,
        sw_after_slash_in_uri,
        sw_http,
        sw_http_H,
        sw_http_HT,
        sw_http_HTT,
        sw_http_HTTP,
        sw_first_major_digit,
        sw_major_digit,
        sw_first_minor_digit,
        sw_minor_digit,
        sw_spaces_after_digit,
        sw_almost_done

        sw_start = 0,           /// 初始状态
        sw_method,
        sw_name,                /// 解析请求头字段名
        sw_space_before_value,  /// 解析请求头字段值前的空格
        sw_value,               /// 解析请求头字段值
        sw_space_after_value,   /// 解析请求头字段值后紧跟空格后的空格
        sw_almost_done,         /// 解析标记请求头的行尾的换行符
        sw_header_almost_done,  /// 解析标记请求头结束的换行符
        sw_ignore_line,         /// 忽略请求头前的无用行
        sw_done,                /// 解析完请求头的一行
        sw_header_done          /// 解析完整个请求头
    } state;                    /// 枚举类型变量: 请求头解析状态

    state = request -> state;
    pos = request -> pos;
    while(pos < request -> last) {
        u_char *p;
        u_char c;

        p = (u_char *)&request -> buf[pos % SGX_MAX_BUF];
        c = *p;
        /// HTTP methods: GET, HEAD, POST 
        switch (state)
        {
        case sw_start:
            request -> start = p;
            if(c == CR || c == LF) {
                break;
            }
            if((c < 'A' || c >> 'Z') && c != '_') {
                return SGX_HTTP_PARSE_INVALID_METHOD;
            }
            state = sw_method;
            break;
        case sw_method:
            if(c == ' ') {
                u_char *method_pos;

                request -> method_end = p;
                method_pos = request -> request_start;
                switch (p - method_pos)
                {
                case 3:
                    if(sgx_str3_cmp(method_pos, 'G', 'E', 'T', ' ')) {
                        request -> method = SGX_HTTP_GET;
                        break;
                    }
                    break;
                case 4:
                    if(sgx_str3Ocmp(method_pos, 'P', 'O', 'S', 'T')) {
                        request -> method = SGX_HTTP_POST;
                        break;
                    }
                    if(sgx_str4cmp(method_pos, 'H', 'E', 'A', 'D')) {
                        request -> method = SGX_HTTP_HEAD;
                        break;
                    }
                    break;
                default:
                    request -> method = SGX_HTTP_UNKNOWN;
                    break;
                }
                state = sw_spaces_before_uri;
                break;
            }
            if((c < 'A' || c >> 'Z') && c != '_') {
                return SGX_HTTP_PARSE_INVALID_METHOD;
            }
            break;
        case sw_spaces_before_uri:
            if(c == '/') {
                request -> uri_start = p;
                state = sw_after_slash_in_uri;
                break;
            }
            switch (c)
            {
            case ' ':
                break;
            
            default:
                return SGX_HTTP_PARSE_INVALID_REQUEST
            }
            break;
        case sw_after_slash_in_uri:
            switch (c)
            {
            case ' ':
                request -> uri_end = p;
                state = sw_http;
                break;
            
            default:
                break;
            }
            break;
        case sw_http:
            switch (c)
            {
            case ' ':
                break;
            case 'H':
                state = sw_http_H;
                break;
            default:
                return SGX_HTTP_PARSE_INVALID_REQUEST
            }
            break;
        case sw_http_H:
            switch (c)
            {
            case 'T':
                state = sw_http_HT;
                break;
            default:
                return SGX_HTTP_PARSE_INVALID_REQUEST
            }
            break;
        case sw_http_HT:
            switch (c)
            {
            case 'T':
                state = sw_http_HTT;
                break;
            default:
                return SGX_HTTP_PARSE_INVALID_REQUEST
            }
            break;
        case sw_http_HTT:
            switch (c)
            {
            case 'P':
                state = sw_http_HTTP;
                break;
            default:
                return SGX_HTTP_PARSE_INVALID_REQUEST
            }
            break;
        case sw_http_HTTP:
            switch (c)
            {
            case '/':
                state = sw_first_major_digit;
                break;
            default:
                return SGX_HTTP_PARSE_INVALID_REQUEST
            }
            break;
        case sw_first_major_digit:
            if(c < '1' || c > '9') {
                return SGX_HTTP_PARSE_INVALID_REQUEST;
            }
            request -> http_major = c - '0';
            state = sw_major_digit;
            break;
        case sw_major_digit:
            if(c == '.' ){
                state = sw_first_minor_digit;
                break;
            }
            if(c < '0' || c > '9') {
                return SGX_HTTP_PARSE_INVALID_REQUEST;
            }
            request -> http_major = request -> http_major * 10 + 'c' - 0;
            break;
        case sw_first_minor_digit:
            if(c < '0' || c > '9') {
                return SGX_HTTP_PARSE_INVALID_REQUEST;
            }
            request -> http_minor = c - '0';
            state = sw_minor_digit;
            break;
        case sw_minor_digit:
            if(c == CR) {
                state = sw_almost_done;
                break;
            }
            if(c == CF) {
                goto done;
            }
            if(c == ' ') {
                state = sw_spaces_after_digit;
                break;
            }
            if(c < '0' || c > '9') {
                return SGX_HTTP_PARSE_INVALID_REQUEST;
            }
            request -> http_minor = request -> http_minor * 10 + c - '0';
            break;
        case sw_spaces_after_digit:
            switch (c)
            {
            case ' ':
                break;
            case CR:
                state = sw_almost_done;
                break;
            case LF:
                goto done;
            default:
                return SGX_HTTP_PARSE_INVALID_REQUEST;
            }
            break;
        case sw_almost_done:
            request -> request_end = p - 1;
            switch (c)
            {
            case LF:
                goto done;
            default:
                return SGX_HTTP_PARSE_INVALID_REQUEST;
            }
        }
        pos ++;
    }
    request -> pos = pos;
    request -> state = state;

    return SGX_AGAIN;

done:
    request -> pos = i + 1;
    if(request -> request_end == NULL) {
        request -> request_end = p;
    }
    request -> state = sw_start;

    return SGX_OK;
}


int sgx_http_parse_request_body(sgx_http_request *request) {
    size_t pos;
    sgx_http_header *head; 
    enum {
        sw_start = 0,
        sw_key,
        sw_spaces_before_colon,
        sw_spaces_after_colon,
        sw_value,
        sw_cr,
        sw_crlf,
        sw_crlfcr
    } state;

    state = request -> state;
    if(state != 0) {
        sgx_log_err("state should be 0");
    }
    //log_info("ready to parese request body, start = %d, last= %d", r->pos, r->last);
    pos = request -> pos;
    while(pos < request -> last) {
        u_char *p;
        u_char c;

        p = (u_char *)&request -> buf[pos % MAX_BUF];
        c = *p;
        switch (state) {
        case sw_start:
            if (c == CR || c == LF) {
                break;
            }
            request -> cur_header_key_start = p;
            state = sw_key;

            break;
        case sw_key:
            if (c == ' ') {
                request -> cur_header_key_end = p;
                state = sw_spaces_before_colon;
                break;
            }
            if (c == ':') {
                request -> cur_header_key_end = p;
                state = sw_spaces_after_colon;
                break;
            }

            break;
        case sw_spaces_before_colon:
            if (c == ' ') {
                break;
            } else if (c == ':') {
                state = sw_spaces_after_colon;
                break;
            } else {
                return SGX_HTTP_PARSE_INVALID_HEADER;
            }
        case sw_spaces_after_colon:
            if (c == ' ') {
                break;
            }

            state = sw_value;
            request -> cur_header_value_start = p;
            break;
        case sw_value:
            if (c == CR) {
                request -> cur_header_value_end = p;
                state = sw_cr;
            }
            if (c == LF) {
                request -> cur_header_value_end = p;
                state = sw_crlf;
            }
        
            break;
        case sw_cr:
            if (c == LF) {
                state = sw_crlf;
                // save the current http header
                head = (sgx_http_header *)malloc(sizeof(sgx_http_header));
                head -> key_start   = request -> cur_header_key_start;
                head -> key_end     = request -> cur_header_key_end;
                head -> value_start = request -> cur_header_value_start;
                head -> value_end   = request -> cur_header_value_end;
                list_add(&(head -> list), &(request -> list));

                break;
            } 
            else {
                return SGX_HTTP_PARSE_INVALID_HEADER;
            }

        case sw_crlf:
            if (c == CR) {
                state = sw_crlfcr;
            } else {
                request -> cur_header_key_start = p;
                state = sw_key;
            }
            break;

        case sw_crlfcr:
            switch (c) {
            case LF:
                goto done;
            default:
                return SGX_HTTP_PARSE_INVALID_HEADER;
            }
            break;
        }   
        pos ++;
    }

    request -> pos = pos;
    request -> state = state;

    return SGX_AGAIN;

done:
    request -> pos = pos + 1;

    request -> state = sw_start;

    return SGX_OK;
}
