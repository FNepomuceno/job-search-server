#include <sqlite3.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct db_conn
{
	sqlite3 * db;
	sqlite3_stmt * res;
	bool is_successful;
} db_conn;

void close_conn(db_conn * conn)
{
	sqlite3_close(conn->db);
	conn->db = NULL;
}

db_conn * new_conn(char * filename)
{
	db_conn * result = malloc(sizeof(db_conn));

	sqlite3 ** db_ref = &(result->db);
	result->res = NULL;
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

void clean_conn(db_conn ** conn_ref)
{
	if (*conn_ref == NULL) return;

	db_conn * conn = *conn_ref;
	close_conn(conn);

	free(conn);
	*conn_ref = NULL;
}

int main()
{
	// -- Connect to database --
	db_conn * conn = new_conn("jobs.db");
	sqlite3 * db = conn->db;
	if (!(conn->is_successful))
	{
		clean_conn(&conn);
		return 1;
	}

	// -- Compile statement --
	sqlite3_stmt * res; // prepared statement
	const char stmt[] = "SELECT SQLITE_VERSION()";

	// Compile statement into a byte-code program
	int query_rc = sqlite3_prepare_v2(
		db,
		stmt,
		sizeof(stmt), // only works with const char[]
		&res,
		NULL
	);

	// If there is an error compiling SQL statement
	if (query_rc != SQLITE_OK)
	{
		// Print error message
		fprintf(stderr, "Failed to fetch data: %s\n",
				sqlite3_errmsg(db));

		// Statement is NULL; no need to finalize it

		// Close the connection
		sqlite3_close(db);

		return 1;
	}


	// -- Execute statement --
	int step_rc = sqlite3_step(res);

	// If there is a new row of data ready to process
	if (step_rc == SQLITE_ROW)
	{
		// Obtain and print text from resulting row in column 0
		printf("%s\n", sqlite3_column_text(res, 0));
	}


	// -- Cleanup --
	// Destroy the prepared statement before closing connection
	sqlite3_finalize(res);

	// Close the connection
	clean_conn(&conn);

	return 0;
}
