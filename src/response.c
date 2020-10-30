#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "response.h"

http_response prepare_raw_text(char * data, int proposed_code)
{
    http_response result;

    result.content_type = TEXT_PLAIN;
    result.status_code = proposed_code;

    int data_len = strlen(data);
    result.content_length = data_len;

    result.content = malloc((data_len+1) * sizeof (char));
    memcpy(result.content, data, data_len);
    result.content[data_len] = '\0';

    return result;
}

http_response load_from_file(char * data, int proposed_code)
{
    http_response result;
    FILE * f = fopen(data, "r");
    if (!f)
    {
        // Replace with loading the 404 page instead TODO
        // Handle the 404 page not working 500 and raw text instead TODO
        result = prepare_raw_text("Page not found", 404);
    }

    // Get the file length
    long length;
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    result.content_length = (int) length;

    // Get the file content
    result.content = malloc(length + 1);
    result.content[length] = '\0';
    fseek(f, 0, SEEK_SET);
    fread(result.content, 1, length, f);
    fclose(f);

    result.status_code = proposed_code;
    result.content_type = TEXT_HTML;

    return result;
}

// Gets the string result of the action
http_response handle_action(web_action * action)
{
    http_response result;

    switch(action->data_type)
    {
    case ACTION_FILE_PATH:
        result = load_from_file(action->data, action->http_code);
        break;
    case ACTION_SQL_QUERY:
        // Execute SQL statement; get data if applicable TODO
        result = prepare_raw_text("This is not a statement, yet", 500);
        break;
    case ACTION_RAW_TEXT:
        result = prepare_raw_text(action->data, action->http_code);
        break;
    default:
        // Behave like ACTION_RAW_TEXT with the following string literal
        result = prepare_raw_text("You shouldn't be seeing this", 500);
        break;
    }

    return result;
}
