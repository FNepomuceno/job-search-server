#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "database.h"
#include "webserver.h"

web_server server;

void sigintHandler(int signum)
{
    // End the web_server loop
    server.is_running = false;
}

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

http_request read_header(int clientfd)
{
    http_request result;
    char buffer[8192];

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
    long count = read(clientfd, buffer, 8191);
    if (count < 0) return result;

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

        // TODO if field is "Content-Length", save the value for STEP 6

        cur_line = next_line + 2;
    }

    // STEP 6: Read body (if exists) TODO
    // char * body_start = header_end + 4;

    // STEP 7: Finalize struct
    result.is_successful = true;
    return result;
}

void web_send_hello(int clientfd, void * extra_data)
{
    // STEP 0: Get database connection from `extra_data`
    db_conn * conn = (db_conn *) extra_data;

    // STEP 1: Read and parse client request
    http_request client_req = read_header(clientfd);
    if (client_req.is_successful)
    {
        printf("METHOD: %s\n", client_req.method);
        printf("URI: %s\n", client_req.uri);
        printf("VERSION: %s\n", client_req.version);
        for (int i = 0; i < client_req.num_lines; i++)
        {
            printf("FIELD: %s\n", client_req.header_lines[2*i]);
            printf("VALUE: %s\n", client_req.header_lines[2*i+1]);
        }
    }

    // Clean up struct
    clean_http_request(&client_req);

    // STEP 2: Handle client request
    // STEP 2a: Obtain data from database as necessary
    // Setup query
    char QUERY[] = "SELECT * FROM number;";
    db_stmt * stmt = new_stmt(conn, QUERY, sizeof(QUERY));

    // Obtain and print results
    printf("Data:\n\n");
    db_row * row = new_row(stmt);
    for (; row->has_value; step_row(row))
    {
        printf("Row:\n");
        for (int i = 0; i < row->num_cols; i++)
        {
            printf("%s\n", (char *)row->values[i]);
        }
        printf("\n");
    }
    printf("End\n");

    // STEP 3: Send response
    // STEP 3a: Load corresponding file
    char * file_buffer = NULL;
    long length;
    FILE * f = fopen("static/html/index.html", "r");
    if (f)
    {
        // Get length
        fseek(f, 0, SEEK_END);
        length = ftell(f);

        // Initialize buffer
        file_buffer = malloc(length+1);
        file_buffer[length] = '\0';
        if (file_buffer)
        {
            // Read file into buffer
            fseek(f, 0, SEEK_SET);
            fread(file_buffer, 1, length, f);
        }

        fclose(f);
    }

    // STEP 3b: Setup response
    char response[] = "HTTP/1.1 200 OK\n"
        "Content-Type: text/html\n"
        "Content-Length: %ld\n"
        "\n"
        "%s";

    char response_buffer[30000];
    sprintf(response_buffer, response, length, file_buffer);

    // STEP 3c: Send response
    write(clientfd, response_buffer, strlen(response_buffer));


    // STEP 4: Clean data
    free(file_buffer);
    clean_row(&row);
    clean_stmt(&stmt);
}

int main()
{
    // Set signal handler
    signal(SIGINT, sigintHandler);

    // Setup database
    char DB_NAME[] = "test.db";
    db_conn * conn = new_conn(DB_NAME);

    // Setup and run server
    int HTTP_PORT = 8080;
    server = new_server(HTTP_PORT, web_send_hello);
    run_server(&server, conn);

    // Clean up
    int result = !(server.is_successful);
    clear_server(&server);
    clean_conn(&conn);

    return result;
}
