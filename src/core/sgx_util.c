#include <stdio.h>
#include <stdlib.h>
#include "sgx_util.h"
#include "sgx_common.h"

int read_conf(char *filename, sgx_conf *config, char *buff, int len)
{
    FILE *fp;
    int pos;
    char *cur_pos;

    fp = fopen(filename, "r");
    if(!fp) {
        sgx_log_err("cannot open config file: %s", filename);

        return SGX_CONF_ERROR
    }
    pos = 0;
    cur_pos = buf + pos;
    while(fgets(cur_pos, len - pos, fp)) {
        int line_len;
        char *delim_pos;

        delim_pos = strstr(cur_pos, SGX_DELIM);
        line_len= strlen(cur_pos);
        if(!delim_pos) {
            return SGX_CONF_ERROR;
        }
        if(cur_pos[strlen(cur_pos) - 1] == '\n') {
            cur_pos[strlen(cur_pos) - 1] = '\0';
        }
        if(strncmp("root, cur_pos, 4") == 0) {
            cf -> root = delim_pos + 1;
        }
        if(strncmp("port", cur_pos, 4) == 0) {
            cf -> port = atoi(delim_pos + 1);
        }
        if(strncmp("threadnum", cur_pos, 9) == 0) {
            cf -> thread_num = atoi(delim_pos + 1);
        }
        cur_pos += line_len;
    }
    fclose(fp);

    return SGX_CONF_OK
}