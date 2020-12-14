#ifndef QUERY_H
#define QUERY_H

#include "vallist.h"
#include "valmap.h"

typedef struct url_detail {
    char * method;
    val_list url;
    val_map query;
} url_detail;

void clear_url_detail(url_detail * detail);
url_detail req_to_detail(char * method, char * url);

val_map query_to_map(char * query_string);

#endif
