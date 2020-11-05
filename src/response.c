#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "response.h"

void clean_http_response(http_response * res)
{
    free(res->content);
}

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

http_response execute_sql_statement(char * data, int proposed_code,
    db_conn * conn)
{
    http_response result;

    // Execute statement and get results
    db_stmt * stmt = new_stmt(conn, data, strlen(data)+1);
    int str_len = 0;

    // Set up dynamically sized array of char *
    int capacity = 10;
    int size = 0;
    char ** rows = malloc(capacity * sizeof (char *));

    // Get column names and column data
    db_row * row = new_row(stmt);
    for (; row->has_value; step_row(row))
    {
        for (int i = 0; i < row->num_cols; i++)
        {
            if (size+1 >= capacity)
            {
                capacity *= 2;
                rows = realloc(rows, capacity * sizeof (char *));
            }

            // Add column name
            int name_size = strlen((char *)row->col_names[i]) + 1;
            rows[size] = malloc(name_size);
            memcpy(rows[size], row->col_names[i], name_size);
            str_len += name_size;

            // Add column data
            int data_size = strlen((char *)row->values[i]) + 1;
            rows[size+1] = malloc(data_size);
            memcpy(rows[size+1], row->values[i], data_size);
            str_len += data_size;

            size += 2;
        }
    }

    // Move data into a single string

    // Calculate how long the resulting string is
    int num_cols = row->num_cols;
    if (num_cols == 0) num_cols = 1;
    int num_results = size / (2 * num_cols);
    int total_len = 14; // initial 14 for bare JSON
    total_len += str_len; // includes strings plus spaces
    total_len += 3 * size; // includes quotes and :/,
    total_len += 4 * num_results; // includes `{}, ` for each result
    total_len -= 2 * num_results; // remove trailing commas in results
    if (num_results > 0) total_len -= 2; // and at the end of the results
    char * res_str = malloc(total_len+1);

    // Beginning
    sprintf(res_str, "{\"result\": [");

    // Results
    int res_offset = 12; // accounts for `{"result": [`
    for (int i = 0; i < num_results; i++)
    {
        sprintf(res_str + res_offset, "{");
        res_offset += 1;
        for (int j = 0; j < num_cols; j++)
        {
            int k = 2*num_cols*i + 2*j; // index into rows
            char * field = rows[k];
            char * value = rows[k+1];
            sprintf(res_str + res_offset, "\"%s\": \"%s\"", field,
                    value);
            res_offset += strlen(field) + strlen(value) + 6;

            if (j + 1 < num_cols)
            {
                sprintf(res_str + res_offset, ", ");
                res_offset += 2;
            }
        }
        sprintf(res_str + res_offset, "}");
        res_offset += 1;

        if (i + 1 < num_results)
        {
            sprintf(res_str + res_offset, ", ");
            res_offset += 2;
        }
    }

    // Ending
    sprintf(res_str + res_offset, "]}");

    // Clean up
    for (int i = 0; i < size; i++)
    {
        free(rows[i]);
    }
    free(rows);
    clean_row(&row);
    clean_stmt(&stmt);

    // Finalize result
    result.content = res_str;
    result.content_type = APPLICATION_JSON;
    result.content_length = total_len;
    result.status_code = proposed_code;
    return result;
}

// Gets the string result of the action
http_response handle_action(web_action * action, db_conn * conn)
{
    http_response result;

    switch(action->data_type)
    {
    case ACTION_FILE_PATH:
        result = load_from_file(action->data, action->http_code);
        break;
    case ACTION_SQL_QUERY:
        result = execute_sql_statement(action->data, action->http_code,
                conn);
        break;
    case ACTION_RAW_TEXT:
        result = prepare_raw_text(action->data, action->http_code);
        break;
    default:
        // Behave like ACTION_RAW_TEXT with the following string literal
        result = prepare_raw_text("You shouldn't be seeing this", 500);
        break;
    }
    clean_web_action(action);

    return result;
}

char * status_code_as_str(int status_code)
{
    char * result = "Bad Request";

    switch (status_code)
    {
    case 200:
        result = "OK";
        break;
    case 404:
        result = "Not Found";
        break;
    case 500:
        result = "Internal Server Error";
        break;
    }

    return result;
}

char * content_type_as_str(content_type type)
{
    char * result;

    switch (type)
    {
    case TEXT_PLAIN:
        result = "text/plain";
        break;
    case TEXT_HTML:
        result = "text/html";
        break;
    case APPLICATION_JSON:
        result = "application/json";
        break;
    default:
        result = "text/plain";
        break;
    }

    return result;
}
