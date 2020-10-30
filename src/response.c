#include <stdlib.h>
#include <string.h>

#include "response.h"

typedef struct
{
    char * data;
    int length;
    content_type type;
} handle_result;

handle_result prepare_raw_text(char * data)
{
    handle_result result;

    result.type = TEXT_PLAIN;
    int data_len = strlen(data);
    result.length = data_len;

    result.data = malloc((data_len+1) * sizeof (char));
    memcpy(result.data, data, data_len);
    result.data[data_len] = '\0';

    return result;
}

// Gets the string result of the action
http_response handle_action(web_action * action)
{
    http_response result;
    result.content = NULL;
    result.content_length = 0;
    result.status_code = 500;

    handle_result content;
    switch(action->data_type)
    {
    case ACTION_FILE_PATH:
        // Read file TODO
        // Replace dummy function TODO
        content = prepare_raw_text("This is not a file, yet");
        break;
    case ACTION_SQL_QUERY:
        // Execute SQL statement; get data if applicable TODO
        break;
    case ACTION_RAW_TEXT:
        content = prepare_raw_text(action->data);
        result.status_code = action->http_code;
        break;
    default:
        // Behave like ACTION_RAW_TEXT with the following string literal
        content = prepare_raw_text("You shouldn't be seeing this");
        break;
    }
    result.content = content.data;
    result.content_type = content.type;
    result.content_length = content.length;

    return result;
}
