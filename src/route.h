#ifndef ROUTE_H
#define ROUTE_H

#include "action.h"
#include "request.h"
#include "valmap.h"

typedef web_action (*handle_func)(val_map *, val_map *, char *);

typedef struct web_route {
    char * method;
    char * path;
    handle_func handler;
} web_route;

web_action web_favicon(val_map * params, val_map * query, char * body);
web_action web_index(val_map * params, val_map * query, char * body);
web_action web_jobs_detail(val_map * params, val_map * query, char * body);
web_action web_jobs_get(val_map * params, val_map * query, char * body);
web_action web_jobs_post(val_map * params, val_map * query, char * body);
web_action web_static(val_map * params, val_map * query, char * body);

#endif
