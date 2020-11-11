#include "database.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

void clear_row(db_row * row)
{
    if (row->has_value)
    {
        row->has_value = false;
        for (int i = 0; i < row->num_cols; i++)
        {
            free(row->values[i]);
            free(row->col_names[i]);
        }
        free(row->values);
        free(row->col_names);
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

        // Get column data
        row->col_types = malloc(sizeof(int)*num_cols);
        row->col_names = malloc(sizeof(char *)*num_cols);
        row->values = malloc(sizeof(void *)*num_cols);
        for (int i = 0; i < num_cols; i++)
        {
            // Column type
            row->col_types[i] = sqlite3_column_type(res, i);

            // Column name
            const char * col_name = sqlite3_column_name(res, i);
            int name_size = strlen((const char *)col_name)+1;

            row->col_names[i] = malloc(name_size);
            memcpy(row->col_names[i], col_name, name_size);

            // Column data (as text)
            const unsigned char * text = sqlite3_column_text(res, i);
            int val_size = strlen((const char *)text)+1;

            row->values[i] = malloc(val_size);
            memcpy(row->values[i], text, val_size);
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

    result->col_names = NULL;
    result->values = NULL;
    result->num_cols = 0;
    result->col_types = NULL;
    result->stmt = stmt;
    result->has_value = true; // to let step work the first time
    result->is_successful = true;

    if (stmt == NULL || !(stmt->is_successful))
    {
        result->is_successful = false;
        result->has_value = false;
    }

    step_row(result);

    return result;
}
