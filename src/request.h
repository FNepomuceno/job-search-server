#ifndef REQUEST_H
#define REQUEST_H

#include <stdbool.h>

#include "valmap.h"

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

void clean_http_request(http_request * req);
void print_request(http_request * req);
http_request read_header(int clientfd);

typedef enum {
    ACTION_FILE_PATH,
    ACTION_SQL_QUERY,
    ACTION_RAW_TEXT,
} action_type;

typedef struct web_action
{
    char * data;
    char * redirect_uri;
    action_type data_type;
    val_map * context;
    int http_code;
    bool clean_data; // true if data is dynamically allocated
} web_action;

void clean_web_action(web_action * action);
web_action interpret_request(http_request * req);

#endif
