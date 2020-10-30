#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "database.h"
#include "request.h"
#include "response.h"
#include "webserver.h"

web_server server;

void sigintHandler(int signum)
{
    // End the web_server loop
    server.is_running = false;
}

void web_send_hello(int clientfd, void * extra_data)
{
    // STEP 0: Get database connection from `extra_data`
    db_conn * conn = (db_conn *) extra_data;

    // STEP 1: Read and parse client request
    http_request client_req = read_header(clientfd);
    if (!client_req.is_successful)
    {
        clean_http_request(&client_req);
        return;
    }
    print_request(&client_req);

    // STEP 2: Interpret client request
    web_action server_action = interpret_request(&client_req);

    // STEP 3: Handle request
    http_response raw_response = handle_action(&server_action);

    // Sample getting data from query
    char QUERY[] = "SELECT * FROM number;";
    db_stmt * stmt = new_stmt(conn, QUERY, sizeof(QUERY));

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

    // STEP 4: Setup response
    // STEP 4a: Get reason phrase from status code
    char * reason_phrase = NULL;
    switch (raw_response.status_code)
    {
    case 200:
        reason_phrase = "OK";
        break;
    case 404:
        reason_phrase = "Not Found";
        break;
    case 500:
        reason_phrase = "Internal Server Error";
        break;
    }

    // STEP 4b: Get type string from content type
    char * type_string;
    switch (raw_response.content_type)
    {
    case TEXT_PLAIN:
        type_string = "text/plain";
        break;
    case TEXT_HTML:
        type_string = "text/html";
        break;
    default:
        type_string = "text/plain";
        break;
    }

    // STEP 4c: Format response
    char response_buffer[30000];
    char response_template[] = "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n\r\n"
        "%s";
    sprintf(response_buffer, response_template, raw_response.status_code,
            reason_phrase, type_string, raw_response.content_length,
            raw_response.content);

    // STEP 5: Send response
    write(clientfd, response_buffer, strlen(response_buffer));


    // STEP 6: Clean data
    clean_http_response(&raw_response);
    clean_http_request(&client_req);
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
