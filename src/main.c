#include <sqlite3.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct db_conn
{
	sqlite3 * db;
	bool is_successful;
} db_conn;

typedef struct db_stmt
{
	sqlite3_stmt * res;
	db_conn * conn;
	char * query;
	int length; // includes null terminator
	bool is_successful;
} db_stmt;

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

int main()
{
	// -- Connect to database --
	db_conn * conn = new_conn("jobs.db");

	// -- Compile statement --
	char query[] = "SELECT SQLITE_VERSION();";
	int length = sizeof(query); // only works with const char[]
	db_stmt * stmt = new_stmt(conn, query, length);
	sqlite3_stmt * res = stmt->res;
	if (!(stmt->is_successful))
	{
		clean_stmt(&stmt);
		clean_conn(&conn);
		return 10;
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
	clean_stmt(&stmt);
	clean_conn(&conn);

	return 0;
}
