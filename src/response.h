#ifndef RESPONSE_H
#define RESPONSE_H

#include "database.h"
#include "request.h"

typedef enum
{
    INVALID,
    TEXT_PLAIN,
    TEXT_HTML,
    TEXT_CSS,
    IMAGE_XICON,
    APPLICATION_JSON
} content_type;

typedef struct http_response
{
    void * content;
    content_type content_type;
    int content_length;
    int status_code;
} http_response;

void clean_http_response(http_response * res);
http_response handle_action(web_action * action, db_conn * conn);

char * status_code_as_str(int status_code);
char * content_type_as_str(content_type type);

#endif
