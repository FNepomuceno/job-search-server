#ifndef ACTION_H
#define ACTION_H

#include <stdbool.h>

#include "request.h"
#include "valmap.h"

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
