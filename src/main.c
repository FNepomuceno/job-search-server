#include <sqlite3.h>
#include <stdio.h>

int main() {
	sqlite3 * db; // database connection

	// Open a connection to the database at `jobs.db`
	int conn_rc = sqlite3_open("jobs.db", &db);

	// If database is not opened/created successfully
	if (conn_rc != SQLITE_OK) {
		// Print error message
		fprintf(stderr, "Cannot open database: %s\n",
				sqlite3_errmsg(db));

		// Close the connection
		sqlite3_close(db);

		return 1;
	}

	// Close the connection when finished
	sqlite3_close(db);

	return 0;
}
