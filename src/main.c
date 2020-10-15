#include <netinet/in.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct db_conn
{
	sqlite3 * db;
	bool is_successful;
} db_conn;

void close_conn(db_conn * conn)
{
	sqlite3_close(conn->db);
	conn->db = NULL;
}

void clean_conn(db_conn ** conn_ref)
{
	if (*conn_ref == NULL) return;

	db_conn * conn = *conn_ref;
	close_conn(conn);

	free(conn);
	*conn_ref = NULL;
}

db_conn * new_conn(char * filename)
{
	db_conn * result = malloc(sizeof(db_conn));

	sqlite3 ** db_ref = &(result->db);
	result->is_successful = true;

	int conn_rc = sqlite3_open(filename, db_ref);
	if (conn_rc != SQLITE_OK)
	{
		result->is_successful = false;

		fprintf(stderr, "Cannot open database: %s\n",
				sqlite3_errmsg(*db_ref));
		close_conn(result);
	}

	return result;
}

typedef struct db_stmt
{
	sqlite3_stmt * res;
	db_conn * conn;
	char * query;
	int length; // includes null terminator
	bool is_successful;
} db_stmt;

void finalize_stmt(db_stmt * stmt)
{
	sqlite3_finalize(stmt->res);
	stmt->res = NULL;
}

void clean_stmt(db_stmt ** stmt_ref)
{
	if (*stmt_ref == NULL) return;

	db_stmt * stmt = *stmt_ref;
	finalize_stmt(stmt);

	free(stmt);
	*stmt_ref = NULL;
}

db_stmt * new_stmt(db_conn * conn, char * query, int length)
{
	db_stmt * result = malloc(sizeof(db_stmt));

	sqlite3_stmt ** res_ref = &(result->res);
	result->conn = conn;
	result->query = query;
	result->length = length;
	result->is_successful = true;

	if (conn == NULL || !(conn->is_successful))
	{
		result->res = NULL;
		result->is_successful = false;
		return result;
	}

	int stmt_rc = sqlite3_prepare_v2(conn->db, query, length,
			res_ref, NULL);
	if (stmt_rc != SQLITE_OK)
	{
		result->is_successful = false;

		fprintf(stderr, "Failed to fetch data: %s\n",
				sqlite3_errmsg(conn->db));
		finalize_stmt(result);
	}

	return result;
}

typedef struct db_row
{
	void ** values;
	int num_cols;
	int * col_types;
	db_stmt * stmt;
	bool has_value; // true only if data is set
	bool is_successful;
} db_row;

void clear_row(db_row * row)
{
	if (row->has_value)
	{
		row->has_value = false;
		for (int i = 0; i < row->num_cols; i++)
		{
			free(row->values[i]);
		}
		free(row->values);
		row->num_cols = 0;
		free(row->col_types);
	}
}

void clean_row(db_row ** row_ref)
{
	if (*row_ref == NULL) return;

	db_row * row = *row_ref;
	clear_row(row);

	free(row);
	*row_ref = NULL;
}

void step_row(db_row * row)
{
	if (!(row->is_successful && row->has_value)) return;
	clear_row(row);

	sqlite3_stmt * res = row->stmt->res;
	int row_rc = sqlite3_step(res);

	if (row_rc == SQLITE_ROW)
	{
		// Get number of columns
		int num_cols = sqlite3_column_count(res);
		row->num_cols = num_cols;

		// Get column types
		row->col_types = malloc(sizeof(int)*num_cols);
		for (int i = 0; i < num_cols; i++)
		{
			row->col_types[i] = sqlite3_column_type(res, i);
		}

		// Get values (extract as text)
		row->values = malloc(sizeof(void *)*num_cols);
		for (int i = 0; i < num_cols; i++)
		{
			const unsigned char * text = sqlite3_column_text(
					res, i);
			int size = strlen((const char *)text)+1;

			void * dest = malloc(sizeof(char)*size);
			memcpy(dest, text, size);

			row->values[i] = dest;
		}

		row->has_value = true;
	}
	else if (row_rc == SQLITE_DONE)
	{
		// Nothing to do here
	}
	else
	{
		// Error here
		row->is_successful = false;

		fprintf(stderr, "Failed to fetch data: %s\n",
				sqlite3_errmsg(row->stmt->conn->db));
	}

	return;
}

db_row * new_row(db_stmt * stmt)
{
	db_row * result = malloc(sizeof(db_row));

	result->values = NULL;
	result->num_cols = 0;
	result->col_types = NULL;
	result->stmt = stmt;
	result->has_value = true; // to let step work the first time
	result->is_successful = true;

	if (stmt == NULL || !(stmt->is_successful))
	{
		result->is_successful = false;
	}

	step_row(result);

	return result;
}

char DB_NAME[] = "test.db";
char QUERY[] = "SELECT * FROM number;";

int HTTP_PORT = 8080;

int main()
{
	// Create socket file descriptor
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == 0)
	{
		perror("In socket");
		exit(EXIT_FAILURE);
	}

	// Make internet-style socket address structure
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(HTTP_PORT);

	memset(address.sin_zero, '\0', sizeof(address.sin_zero));

	// Bind address to socket
	if (bind(server_fd, (struct sockaddr *)&address,
		sizeof(address)) < 0)
	{
		perror("In bind");
		exit(EXIT_FAILURE);
	}

	// Open socket for requests
	if (listen(server_fd, 1024) < 0)
	{
		perror("In listen");
		exit(EXIT_FAILURE);
	}

	// HTTP Response Message has a header and a body
	// separated by an empty line
	char * response = "HTTP/1.1 200 OK\n"
		"Content-Type: text/plain\n"
		"Content-Length: 12\n"
		"\n"
		"Hello world!";

	// Loop
	while(1)
	{
		printf("\n+++ Waiting for connections +++\n\n");
		int new_socket = accept(
			server_fd,
			(struct sockaddr *)&address,
			(socklen_t *)&addrlen
		);
		if (new_socket < 0)
		{
			perror("In socket");
			exit(EXIT_FAILURE);
		}

		// Read HTTP Request
		char buffer[30000] = {0};
		long req_length = read(new_socket, buffer, 30000);
		if (req_length < 0)
		{
			perror("In read");
			close(new_socket);
			continue;
		}

		// HTTP Response goes here
		printf("%s\n", buffer);
		write(new_socket, response, strlen(response));
		printf("--- Message sent ---\n");

		// Done with connection
		close(new_socket);
	}

	return 0;
}
