#ifndef RESPONSE_H
#define RESPONSE_H

#include "request.h"

typedef enum
{
    INVALID,
    TEXT_PLAIN,
    TEXT_HTML
} content_type;

typedef struct http_response
{
    char * content;
    content_type content_type;
    int content_length;
    int status_code;
} http_response;

http_response handle_action(web_action * action);

#endif
