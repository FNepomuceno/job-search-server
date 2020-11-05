#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "request.h"

void clean_http_request(http_request * req)
{
    for (int i = 0; i < 2 * req->num_lines; i++)
    {
        free(req->header_lines[i]);
    }

    free(req->method);
    free(req->uri);
    free(req->version);
    free(req->body);
    free(req->header_lines);
}

void print_request(http_request * req)
{
    if (!req->is_successful)
    {
        printf("INVALID HTTP REQUEST\n");
        return;
    }

    printf("METHOD: %s\n", req->method);
    printf("URI: %s\n", req->uri);
    printf("VERSION: %s\n", req->version);
    for (int i = 0; i < req->num_lines; i++)
    {
        printf("FIELD: %s\n", req->header_lines[2*i]);
        printf("VALUE: %s\n", req->header_lines[2*i+1]);
    }
    printf("BODY: %s\n", req->body);
}

http_request read_header(int clientfd)
{
#define REQ_BUFFER_LEN 8192
    http_request result;
    char buffer[REQ_BUFFER_LEN];

    // STEP 0: Provide dummy values for struct
    result.method = NULL;
    result.uri = NULL;
    result.version = NULL;
    result.body = NULL;
    result.header_lines = NULL;
    result.num_lines = 0;
    result.body_length = 0;
    result.is_successful = false;

    // STEP 1: Get request from client
    // - Reject if read is not successful
    long count = read(clientfd, buffer, REQ_BUFFER_LEN-1);
    if (count < 0) return result;
    buffer[count] = '\0';

    // STEP 2: Find where header ends
    // - Reject request if header does not fit within buffer
    char * header_end = strstr(buffer, "\r\n\r\n");
    if (header_end == NULL)
    {
        fprintf(stderr, "Header too large! Received:\n");
        fprintf(stderr, "%s\n", buffer);
        return result;
    }
    long header_len = header_end - buffer;

    // STEP 3: Set up the header lines
    // STEP 3a: Find how many header lines the request has
    char * cur_line = buffer;
    long total_length = 0;
    int line_num = 0;

    while (total_length < header_len)
    {
        char * next_line = strstr(cur_line, "\r\n");
        if (next_line == NULL) break;

        total_length += next_line - cur_line + 2;
        cur_line = next_line + 2;
        ++line_num;
    }

    // STEP 3b: Set up relevant data in the struct
    // - The line count ignores the first (request-line) line
    // - Header lines are double the number of lines to split apart the
    //   header field and the values
    --line_num;
    result.num_lines = line_num;
    result.header_lines = malloc(2 * line_num * sizeof (char *));
    for (int i = 0; i < 2 * line_num; i++)
    {
        result.header_lines[i] = NULL;
    }

    // STEP 4: Parse the request line
    char * request_line = buffer;

    // STEP 4a: Get the HTTP method
    char * method_end = strstr(request_line, " ");
    if (method_end == NULL) return result;

    long method_len = method_end - request_line;
    result.method = malloc((method_len+1) * sizeof (char));
    memcpy(result.method, buffer, method_len);
    result.method[method_len] = '\0';

    // STEP 4b: Get the request URI
    char * uri_end = strstr(method_end + 1, " ");
    if (uri_end == NULL) return result;

    long uri_len = uri_end - method_end - 1;
    result.uri = malloc((uri_len+1) * sizeof (char));
    memcpy(result.uri, method_end+1, uri_len);
    result.uri[uri_len] = '\0';

    // STEP 4c: Get the HTTP version
    char * version_end = strstr(uri_end + 1, "\r\n");
    if (version_end == NULL) return result;

    long version_len = version_end - uri_end - 1;
    result.version = malloc((version_len+1) * sizeof (char));
    memcpy(result.version, uri_end+1, version_len);
    result.version[version_len] = '\0';

    // STEP 5: Read header lines
    cur_line = strstr(buffer, "\r\n") + 2;
    long body_length = 0;
    for (int i = 0; i < result.num_lines; i++)
    {
        char * next_line = strstr(cur_line, "\r\n");
        char * field_end = strstr(cur_line, ": ");
        if (field_end == NULL) return result;

        // STEP 5a: Get the field
        long field_len = field_end - cur_line;
        result.header_lines[2*i] = malloc((field_len+1) * sizeof (char));
        memcpy(result.header_lines[2*i], cur_line, field_len);
        result.header_lines[2*i][field_len] = '\0';

        // STEP 5b: Get the value
        long value_len = next_line - field_end - 2;
        result.header_lines[2*i+1] = malloc((value_len+1) * sizeof (char));
        memcpy(result.header_lines[2*i+1], field_end+2, value_len);
        result.header_lines[2*i+1][value_len] = '\0';

        // STEP 5c: Get the size of the body
        // - Ignore the "Transfer-Encoding" header rule
        if (strcmp(result.header_lines[2*i], "Content-Length") == 0)
        {
            body_length = strtoimax(result.header_lines[2*i+1], NULL, 10);
        }

        cur_line = next_line + 2;
    }

    // STEP 6: Read body
    char * body_start = header_end + 4;
    result.body = malloc((body_length+1) * sizeof (char));
    result.body[body_length] = '\0';

    long body_chunk_len = count - (body_start - buffer);
    memcpy(result.body, body_start, body_chunk_len);

    long index = body_chunk_len;
    while(index < body_length)
    {
        long read_len = read(clientfd, buffer, REQ_BUFFER_LEN-1);
        memcpy(result.body + index, buffer, read_len);

        index += read_len;
    }

    // STEP 7: Finalize struct
    result.is_successful = true;
    return result;
}

void clean_web_action(web_action * action)
{
    if (action->clean_data)
    {
        free(action->data);
    }
}

web_action interpret_request(http_request * req)
{
    web_action result;
    result.data = "static/html/not_found.html";
    result.data_type = ACTION_FILE_PATH;
    result.http_code = 404;
    result.clean_data = false;

    // Malformed request
    if (!req->is_successful) {
        result.data = "You dun messed up";
        result.data_type = ACTION_RAW_TEXT;
        result.http_code = 400;
        return result;
    }

    // "GET /favicon.ico" (icon)
    if (strcmp(req->method, "GET") == 0
            && strcmp(req->uri, "/favicon.ico") == 0)
    {
        result.data = "favicon.ico";
        result.data_type = ACTION_FILE_PATH;
        result.http_code = 200;
    }

    // "GET /" (web)
    if (strcmp(req->method, "GET") == 0 && strcmp(req->uri, "/") == 0)
    {
        result.data = "static/html/index.html";
        result.data_type = ACTION_FILE_PATH;
        result.http_code = 200;
    }

    // "GET /static/*" (static files)
    if (strcmp(req->method, "GET") == 0
            && strncmp(req->uri, "/static/", 8) == 0)
    {
        char * suffix = req->uri + 8;
        int data_len = 7 + strlen(suffix);
        char * data = malloc(data_len + 1);
        sprintf(data, "static/%s", suffix);

        result.data = data;
        result.data_type = ACTION_FILE_PATH;
        result.http_code = 200;
        result.clean_data = true;
    }

    // "GET /jobs" (api)
    if (strcmp(req->method, "GET") == 0
            && strncmp(req->uri, "/jobs", 4) == 0)
    {
        // No extra queries case: get all columns from 'jobs' table
        // Make sure nothing follows "/jobs" in URI TODO
        result.data = "SELECT * FROM jobs;";
        result.data_type = ACTION_SQL_QUERY;
        result.http_code = 200;

        // Handle other query cases TODO
    }

    // "POST /jobs/new" (api)
    if (strcmp(req->method, "POST") == 0
            && strcmp(req->uri, "/jobs/new") == 0)
    {
        // Turn body into a SQL query TODO
    }

    return result;
}
