#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "database.h"

typedef struct web_server
{
	int server_fd;
	struct sockaddr_in address;
	int addrlen;
	bool is_successful;
	void (*handle_client)(int);
} web_server;

void clear_server(web_server * server)
{
	// Close the socket
	close(server->server_fd);
	server->server_fd = 0;
}

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

web_server new_server(int port, void (*handle_client)(int))
{
	web_server result;
	result.is_successful = true;
	result.handle_client = handle_client;

	// Create socket file descriptor
	result.server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (result.server_fd == 0)
	{
		perror("Could not open socket");
		result.is_successful = false;
		return result;
	}

	// Make internet-style socket address structure
	result.addrlen = sizeof(result.address);

	result.address.sin_family = AF_INET;
	result.address.sin_addr.s_addr = INADDR_ANY;
	result.address.sin_port = htons(port);

	memset(result.address.sin_zero, '\0',
		sizeof(result.address.sin_zero));

	// Bind address to socket
	if (bind(result.server_fd,
		(struct sockaddr *)&result.address,
		sizeof(result.address)) < 0)
	{
		perror("Could not bind address");
		result.is_successful = false;
		return result;
	}

	// Open socket for requests
	if (listen(result.server_fd, 1024) < 0)
	{
		perror("Problem with listen");
		result.is_successful = false;
		return result;
	}

	// Resulting socket ready for response loop
	return result;
}

void run_server(web_server * server)
{
	if (!(server->is_successful)) return;

	while(true)
	{
		int clientfd = accept(
			server->server_fd,
			(struct sockaddr *)&(server->address),
			(socklen_t *)&(server->addrlen)
		);
		if (clientfd < 0)
		{
			perror("Could not accept connection");
			server->is_successful = false;
			return;
		}

		(server->handle_client)(clientfd);
		close(clientfd);
	}
}

char DB_NAME[] = "test.db";
char QUERY[] = "SELECT * FROM number;";

int HTTP_PORT = 8080;

int main()
{
	// Connect to database
	db_conn * conn = new_conn(DB_NAME);

	// Evaluate statement
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

	// Cleanup
	int result = !(row->is_successful);
	clean_row(&row);
	clean_stmt(&stmt);
	clean_conn(&conn);

	return result;
}
