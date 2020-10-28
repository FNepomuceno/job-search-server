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

void web_send_hello(int clientfd, void * extra_data)
{
    // STEP 0: Get database connection from `extra_data`
    db_conn * conn = (db_conn *) extra_data;

    // STEP 1: Read and parse client request
    // STEP 1a: Read header
    char buffer[8192] = {0};
    long count = read(clientfd, buffer, 8191);
    if (count < 0)
    {
        perror("In read");
        return;
    }

    // STEP 1b: Find where header ends
    char * header_end = strstr(buffer, "\r\n\r\n");
    if (header_end == NULL)
    {
        fprintf(stderr, "Header too large! Received:\n");
        fprintf(stderr, "%s\n", buffer);
    }
    long header_len = header_end - buffer;

    // STEP 1c: Parse header
    char line_end[] = "\r\n";

    char * cur_line = buffer;
    long total_length = 0;
    int line_num = 0;

    while (total_length < header_len)
    {
        char * next_line = strstr(cur_line, line_end);
        long cur_length = next_line - cur_line;
        if (next_line == NULL) break;

        if (line_num == 0) // First line is the request-line
        {
            printf("Request-line: %.*s\n", (int)cur_length, cur_line);
        }
        else // Next several lines are the header-lines
        {
            char separator[] = ": ";
            char * value = strstr(cur_line, separator) + 2;
            long field_length = value - 2 - cur_line;
            printf("Field: %.*s\n", (int)field_length, cur_line);
            printf(
                "Value: %.*s\n",
                (int)(cur_length-2-field_length),
                value
            );
        }
        total_length += cur_length + 2;
        cur_line = next_line + 2;
        line_num += 1;
    }
    printf("Total length: %ld\n", total_length);

    // STEP 1d: Obtain body if exists

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
