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
    http_response raw_response = handle_action(&server_action, conn);

    // STEP 4: Setup response
    // STEP 4a: Get remaining required data for template
    char * reason_phrase = status_code_as_str(raw_response.status_code);
    char * type_string = content_type_as_str(raw_response.content_type);
    int header_size = 0;

    // STEP 4b: Format response
    char response_buffer[30000];
    if (raw_response.status_code / 100 == 3) // Redirect 3xx
    {
        char response_template[] = "HTTP/1.1 %d %s\r\n"
            "Location: %s\r\n\r\n";
        sprintf(response_buffer, response_template,
                raw_response.status_code, reason_phrase,
                raw_response.content);

        header_size = strlen(response_buffer);
    }
    else
    {
        char response_template[] = "HTTP/1.1 %d %s\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %d\r\n\r\n";
        sprintf(response_buffer, response_template,
                raw_response.status_code, reason_phrase, type_string,
                raw_response.content_length);

        header_size = strlen(response_buffer);
        memcpy(response_buffer + header_size, raw_response.content,
                raw_response.content_length);
    }

    // STEP 5: Send response
    write(clientfd, response_buffer,
            header_size + raw_response.content_length);


    // STEP 6: Clean data
    clean_http_response(&raw_response);
    clean_http_request(&client_req);
}

int main()
{
    // Set signal handler
    signal(SIGINT, sigintHandler);

    // Setup database
    char DB_NAME[] = "jobs.db";
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
