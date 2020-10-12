#include <sqlite3.h>
#include <stdio.h>

int main()
{
	// -- Connect to database --
	sqlite3 * db; // database connection

	// Open a connection to the database at `jobs.db`
	int conn_rc = sqlite3_open("jobs.db", &db);

	// If database is not opened/created successfully
	if (conn_rc != SQLITE_OK)
	{
		// Print error message
		fprintf(stderr, "Cannot open database: %s\n",
				sqlite3_errmsg(db));

		// Close the connection
		sqlite3_close(db);

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

	// Close the connection when finished
	sqlite3_close(db);

	return 0;
}
