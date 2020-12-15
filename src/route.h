#ifndef ROUTE_H
#define ROUTE_H

#include "request.h"
#include "valmap.h"

typedef struct web_route {
    char * method;
    char * path;
    web_action (*handler)(val_map *);
} web_route;

web_action web_index(val_map * params);

#endif
