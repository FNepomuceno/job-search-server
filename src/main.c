#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "database.h"
#include "request.h"
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
    printf("METHOD: %s\n", client_req.method);
    printf("URI: %s\n", client_req.uri);
    printf("VERSION: %s\n", client_req.version);
    for (int i = 0; i < client_req.num_lines; i++)
    {
        printf("FIELD: %s\n", client_req.header_lines[2*i]);
        printf("VALUE: %s\n", client_req.header_lines[2*i+1]);
    }
    printf("BODY: %s\n", client_req.body);

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
    char response_buffer[30000];
    if (strcmp(client_req.method, "GET") == 0
        && strcmp(client_req.uri, "/") == 0)
    {
        char response[] = "HTTP/1.1 200 OK\n"
            "Content-Type: text/html\n"
            "Content-Length: %ld\n"
            "\n"
            "%s";
        sprintf(response_buffer, response, length, file_buffer);
    }
    else
    {
        char response[] = "HTTP/1.1 404 Not Found\n"
            "Content-Type: text/plain\n"
            "Content-Length: 4\n"
            "\n"
            "Nope";
        sprintf(response_buffer, response, length, file_buffer);
    }

    // STEP 3c: Send response
    write(clientfd, response_buffer, strlen(response_buffer));


    // STEP 4: Clean data
    clean_http_request(&client_req);
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
