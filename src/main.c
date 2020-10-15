#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "database.h"
#include "webserver.h"

void web_send_hello(int clientfd)
{
	// HTTP Response Message has a header and a body
	// separated by an empty line
	char * response = "HTTP/1.1 200 OK\n"
		"Content-Type: text/plain\n"
		"Content-Length: 12\n"
		"\n"
		"Hello world!";

	char buffer[30000] = {0};
	long req_length = read(clientfd, buffer, 30000);
	if (req_length < 0)
	{
		perror("In read");
		return;
	}

	// HTTP Response goes here
	printf("%s\n", buffer);
	write(clientfd, response, strlen(response));
}

char DB_NAME[] = "test.db";
char QUERY[] = "SELECT * FROM number;";

int HTTP_PORT = 8080;

int main()
{
	// Setup database and query
	db_conn * conn = new_conn(DB_NAME);
	db_stmt * stmt = new_stmt(conn, QUERY, sizeof(QUERY));

	// Print results
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

	// Setup and run server
	web_server server = new_server(HTTP_PORT, web_send_hello);
	run_server(&server);

	// Should not reach here except on error
	clear_server(&server);
	clean_row(&row);
	clean_stmt(&stmt);
	clean_conn(&conn);

	return 1;
}
