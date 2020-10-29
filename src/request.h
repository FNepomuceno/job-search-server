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

void clean_http_request(http_request * req);
http_request read_header(int clientfd);

#endif
