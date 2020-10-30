#ifndef REQUEST_H
#define REQUEST_H

#include <stdbool.h>

typedef struct http_request
{
    char * method;
    char * uri;
    char * version;
    char * body;
    char ** header_lines;
    int num_lines;
    int body_length;
    bool is_successful;
} http_request;

typedef enum {
    ACTION_FILE_PATH,
    ACTION_SQL_QUERY,
    ACTION_RAW_TEXT,
} action_type;

typedef struct web_action
{
    char * data;
    action_type data_type;
    int http_code;
} web_action;

void clean_http_request(http_request * req);
void print_request(http_request * req);
http_request read_header(int clientfd);

web_action interpret_request(http_request * req);

#endif