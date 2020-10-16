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
	printf("Closing server");
	server.is_running = false;
}

void web_send_hello(int clientfd, void * extra_data)
{
	// STEP 0: Get database connection from `extra_data`
	db_conn * conn = (db_conn *) extra_data;

	// STEP 1: Read and parse client request
	char buffer[30000] = {0};
	long req_length = read(clientfd, buffer, 30000);
	if (req_length < 0)
	{
		perror("In read");
		return;
	}
	printf("%s\n", buffer);

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
	FILE * f = fopen("site/index.html", "r");
	if (f)
	{
		// Get length
		fseek(f, 0, SEEK_END);
		length = ftell(f);

		// Initialize buffer
		file_buffer = malloc(length);
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

	sprintf(buffer, response, length, file_buffer);

	// STEP 3c: Send response
	write(clientfd, buffer, strlen(buffer));


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
