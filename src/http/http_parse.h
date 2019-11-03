#ifndef HTTP_PARSE_H
#define HTTP_PARSE_H

#define CR '\r'
#define LF '\n'
#define CRLFCRLF "\r\n\r\n"

int sgx_http_parse_request_line(sgx_http_request *request);
int sgx_http_parse_request_body(sgx_http_request *request);

#endif