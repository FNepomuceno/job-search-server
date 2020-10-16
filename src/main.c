#include <signal.h>
#include <stdio.h>
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
	// Obtain DB connection from extra_data
	db_conn * conn = (db_conn *) extra_data;


	// Read request from client
	char buffer[30000] = {0};
	long req_length = read(clientfd, buffer, 30000);
	if (req_length < 0)
	{
		perror("In read");
		return;
	}


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


	// HTTP Response Message has a header and a body
	// separated by an empty line
	char * response = "HTTP/1.1 200 OK\n"
		"Content-Type: text/plain\n"
		"Content-Length: 12\n"
		"\n"
		"Hello world!";

	// HTTP Response goes here
	printf("%s\n", buffer);
	write(clientfd, response, strlen(response));


	// Clean DB data at the end
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

	// Should not reach here except on error
	clear_server(&server);
	clean_conn(&conn);

	return 1;
}
