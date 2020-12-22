#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "query.h"
#include "route.h"
#include "vallist.h"
#include "valmap.h"

web_action web_favicon(val_map * params, val_map * query, char * body)
{
    web_action result;
    result.redirect_uri = "";
    result.clean_data = false;
    result.context = NULL;

    result.data = "favicon.ico";
    result.data_type = ACTION_FILE_PATH;
    result.http_code = 200;

    return result;
}

web_action web_index(val_map * params, val_map * query, char * body)
{
    web_action result;
    result.redirect_uri = "";
    result.clean_data = false;
    result.context = NULL;

    result.data = "static/html/index.html";
    result.data_type = ACTION_FILE_PATH;
    result.http_code = 200;

    return result;
}

web_action web_invalid(val_map * params, val_map * query, char * body)
{
    web_action result;
    result.redirect_uri = "";
    result.clean_data = false;
    result.context = NULL;

    result.data = "Something horribly wrong happened";
    result.data_type = ACTION_RAW_TEXT;
    result.http_code = 500;

    return result;
}

web_action web_jobs_detail(val_map * params, val_map * query, char * body)
{
    web_action result;
    result.redirect_uri = "";
    result.clean_data = false;

    char * job_id = map_get(params, "job_id");

    result.context = malloc(sizeof (val_map));
    *result.context = new_map();
    insert_entry(result.context, "job_id", 6, job_id, strlen(job_id));

    // Finalize request
    result.data = "static/html/jobs_detail.html";
    result.data_type = ACTION_FILE_PATH;
    result.http_code = 200;

    return result;
}

web_action web_jobs_get(val_map * params, val_map * query, char * body)
{
    web_action result;
    result.redirect_uri = "";
    result.context = NULL;

    // Base query
    int num_filters = 0;
    val_list query_values = new_list();
    list_insert(&query_values, "SELECT * FROM jobs", 18);

    // The first joining word is "WHERE"
    // The following joining words are "AND"

    // "updated-after"
    char * ua_val = map_get(query, "updated-after");
    if (ua_val != NULL)
    {
        list_insert(&query_values, " WHERE latest_update >= \"", 25);
        list_insert(&query_values, ua_val, strlen(ua_val));
        list_insert(&query_values, "\"", 1);
        ++num_filters;
    }

    // "updated-before"
    char * ub_val = map_get(query, "updated-before");
    if (ub_val != NULL)
    {
        if (num_filters == 0)
        {
            list_insert(&query_values, " WHERE ", 7);
        }
        else { list_insert(&query_values, " AND ", 5); }

        list_insert(&query_values, "latest_update <= \"", 18);
        list_insert(&query_values, ub_val, strlen(ub_val));
        list_insert(&query_values, "\"", 1);
        ++num_filters;
    }

    // These query values do not filter, but change the ordering
    char * order_val = map_get(query, "order-by");
    char * dir_val = map_get(query, "order-direction");

    if (order_val == NULL) { order_val = "date_applied"; }
    if (dir_val == NULL)
    {
        if (strcmp(order_val, "date_applied") == 0 ||
            strcmp(order_val, "latest_update") == 0)
        {
            dir_val = "DESC";
        }
        else { dir_val = "ASC"; }
    }

    list_insert(&query_values, " ORDER BY ", 10);
    list_insert(&query_values, order_val, strlen(order_val));
    list_insert(&query_values, " ", 1);
    list_insert(&query_values, dir_val, strlen(dir_val));

    // Insert ending ";"
    list_insert(&query_values, ";", 1);

    // Get string length
    long string_length = 0;
    for (int i = 0; i < query_values.size; ++i)
    {
        string_length += strlen(query_values.values[i]);
    }

    // Set up query string
    char * sql_query = malloc(string_length + 1);
    char * q_cursor = sql_query;
    for (int i = 0; i < query_values.size; ++i)
    {
        long len = strlen(query_values.values[i]);
        memcpy(q_cursor, query_values.values[i], len);
        q_cursor += len;
    }
    sql_query[string_length] = '\0';

    // Cleanup
    clear_val_list(&query_values);

    // Finalize request
    result.data = sql_query;
    result.data_type = ACTION_SQL_QUERY;
    result.http_code = 200;
    result.clean_data = true;

    return result;
}

web_action web_jobs_post(val_map * params, val_map * query, char * body)
{
    web_action result;
    result.redirect_uri = "";
    result.clean_data = false;
    result.context = NULL;

    val_map map = query_to_map(body);

    // All values to be inserted into the database
    char * values[10];

    // Get required fields and their indices
    values[0] = map_get(&map, "company");
    values[1] = map_get(&map, "position");
    values[2] = map_get(&map, "location");
    values[3] = map_get(&map, "app_link");
    values[4] = map_get(&map, "app_method");
    values[5] = map_get(&map, "referrer");
    values[6] = map_get(&map, "version");

    // Verify values to be non-empty TODO
    if (values[0] != NULL && values[1] != NULL && values[2] != NULL
        && values[3] != NULL && values[4] != NULL && values[5] != NULL
        && values[6] != NULL)
    {
        values[7] = "Pending"; // Status
        values[8] = "Applied"; // Progress
        values[9] = "None"; // Interview Details

        // Turn body into a SQL query
        char * template_query = "INSERT INTO jobs (company, "
            "position, location, app_link, apply_method, "
            "referrer, resume_version, status, progress, "
            "interview_details) VALUES (\"%s\", \"%s\", \"%s\", "
            "\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", "
            "\"%s\");";
        int stmt_len = strlen(template_query);
        for (int i = 0; i < 10; ++i)
        {
            stmt_len += strlen(values[i]) - 2;
        }

        char *statement = malloc(stmt_len + 1);
        sprintf(statement, template_query, values[0], values[1],
            values[2], values[3], values[4], values[5], values[6],
            values[7], values[8], values[9]);
        statement[stmt_len] = '\0';

        result.data = statement;
        result.redirect_uri = "/";
        result.data_type = ACTION_SQL_QUERY;
        result.http_code = 303;
        result.clean_data = true;
    }
    else
    {
        // Malformed Request
        result.data = "Some required fields are missing";
        result.data_type = ACTION_RAW_TEXT;
        result.http_code = 400;
    }

    // Cleanup
    clear_val_map(&map);

    return result;
}

web_action web_not_found(val_map * params, val_map * query, char * body)
{
    web_action result;
    result.redirect_uri = "";
    result.clean_data = false;
    result.context = NULL;

    result.data = "static/html/not_found.html";
    result.data_type = ACTION_FILE_PATH;
    result.http_code = 404;

    return result;
}

web_action web_static(val_map * params, val_map * query, char * body)
{
    web_action result;
    result.redirect_uri = "";
    result.context = NULL;

    char * sub_path = map_get(params, "*");
    long length = strlen(sub_path) + 7;

    char * file_path = malloc(length + 1);
    sprintf(file_path, "static/%s", sub_path);
    file_path[length] = '\0';

    result.data = file_path;
    result.data_type = ACTION_FILE_PATH;
    result.http_code = 200;
    result.clean_data = true;

    return result;
}
