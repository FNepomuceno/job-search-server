#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "query.h"
#include "route.h"
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

web_action web_jobs_detail(val_map * params, val_map * query, char * body)
{
    web_action result;
    result.redirect_uri = "";
    result.clean_data = false;

    // Add params to context TODO
    result.context = NULL;
    printf("\"%s\" not available yet\n", map_get(params, "job_id"));

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

    // Setup
    int qindex = 0;
    int query_length = 0;
    // Use value list instead of char *[] TODO
    char * qvalues[30]; // 30, because it should be enough

    // Base query
    qvalues[0] = "SELECT * FROM jobs";
    query_length += 18;
    ++qindex;

    // "updated-after"
    char * ua_val = map_get(query, "updated-after");
    if (ua_val != NULL)
    {
        // Always first so only add " where "
        qvalues[qindex] = " WHERE latest_update >= \"";
        qvalues[qindex+1] = ua_val;
        qvalues[qindex+2] = "\"";
        query_length += 26 + strlen(ua_val);
        qindex += 3;
    }

    // "updated-before"
    char * ub_val = map_get(query, "updated-before");
    if (ub_val != NULL)
    {
        // Need to check if is first or not
        if (qindex == 1)
        {
            qvalues[qindex] = " WHERE ";
            query_length += 7;
        }
        else
        {
            qvalues[qindex] = " AND ";
            query_length += 5;
        }
        qvalues[qindex+1] = "latest_update <= \"";
        qvalues[qindex+2] = ub_val;
        qvalues[qindex+3] = "\"";
        query_length += 19 + strlen(ub_val);
        qindex += 4;
    }

    // These query values do not filter, but change the ordering
    // implement ordering TODO
    // "order-direction"
    // "order-by"

    // Set up query string
    char * sql_query = malloc(query_length + 10);
    char * q_cursor = sql_query;
    for (int i = 0; i < qindex; i++)
    {
        int len = strlen(qvalues[i]);
        memcpy(q_cursor, qvalues[i], len);
        q_cursor += len;
    }
    strcpy(q_cursor, ";");

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
        for (int i = 0; i < 10; i++)
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
