#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <stdbool.h>

typedef struct db_conn
{
    sqlite3 * db;
    bool is_successful;
} db_conn;

void close_conn(db_conn * conn);
void clean_conn(db_conn ** conn_ref);
db_conn * new_conn(char * filename);

typedef struct db_stmt
{
    sqlite3_stmt * res;
    db_conn * conn;
    char * query;
    int length; // includes null terminator
    bool is_successful;
} db_stmt;

void finalize_stmt(db_stmt * stmt);
void clean_stmt(db_stmt ** stmt_ref);
db_stmt * new_stmt(db_conn * conn, char * query, int length);

typedef struct db_row
{
    void ** values;
    int num_cols;
    int * col_types;
    db_stmt * stmt;
    bool has_value; // true only if data is set
    bool is_successful;
} db_row;

void clear_row(db_row * row);
void clean_row(db_row ** row_ref);
void step_row(db_row * row);
db_row * new_row(db_stmt * stmt);

#endif
