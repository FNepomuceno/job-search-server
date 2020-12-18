#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "action.h"
#include "route.h"
#include "query.h"
#include "valmap.h"

void clean_web_action(web_action * action)
{
    if (action->clean_data)
    {
        free(action->data);
    }
}

val_map * match_uri(char * met, char * temp, url_detail * req_detail)
{
    val_map * result = malloc(sizeof (val_map));
    *result = new_map();

    // Template rules:
    // - No queries are allowed in the template
    // - Each section of path has a string to match or a variable
    // - Variables are indicated with {}s enclosing its name
    // - Paths can end in * to allow sub-paths
    //   - This sub-path can be found in the variable "*" of the result
    //   - Template paths that don't end in * require the request's url
    //     to match in size with the template's path size
    // - The following code assumes the template follows these rules
    url_detail temp_detail = req_to_detail(met, temp);

    if (strcmp(met, req_detail->method) != 0 ||
        temp_detail.url.size > req_detail->url.size)
    {
        clear_url_detail(&temp_detail);
        clear_val_map(result);
        free(result);
        return NULL;
    }

    for (int i = 0; i < temp_detail.url.size; i++)
    {
        char * temp_sec = temp_detail.url.values[i];
        char * req_sec = req_detail->url.values[i];
        // Check for variables
        if (temp_sec[0] == '{')
        {
            insert_entry(result, temp_sec+1, strlen(temp_sec)-2,
                req_sec, strlen(req_sec));
        }
        else if (strcmp(temp_sec, "*") == 0)
        {
            // Compute sub path and put it in result TODO
            break;
        }
        else if (strcmp(temp_sec, req_sec) != 0)
        {
            clear_url_detail(&temp_detail);
            clear_val_map(result);
            free(result);
            return NULL;
        }
    }

    // Cleanup
    clear_url_detail(&temp_detail);

    return result;
}

web_action interpret_request(http_request * req)
{
    web_action result;
    result.data = "static/html/not_found.html";
    result.redirect_uri = "";
    result.data_type = ACTION_FILE_PATH;
    result.http_code = 404;
    result.clean_data = false;

    // Malformed request
    if (!req->is_successful) {
        result.data = "Something horribly wrong happened";
        result.data_type = ACTION_RAW_TEXT;
        result.http_code = 400;
        return result;
    }

    url_detail req_detail = req_to_detail(req->method, req->uri);

    web_route routes[] = {
        { "GET", "/favicon.ico", web_favicon },
        { "GET", "/", web_index },
        // { "GET",  "/static/*", web_static },
        { "GET", "/jobs/{job_id}", web_jobs_detail },
        { "GET", "/api/jobs", web_jobs_get },
        { "POST", "/api/jobs", web_jobs_post }
    };

    long num_routes = sizeof (routes) / sizeof (web_route);
    val_map * match = NULL;
    for (int i = 0; i < num_routes; i++)
    {
        match = match_uri(routes[i].method, routes[i].path, &req_detail);
        if (match != NULL) {
            web_action result =
                routes[i].handler(match, &req_detail.query, req->body);

            // Cleanup
            clear_val_map(match);
            free(match);
            clear_url_detail(&req_detail);

            return result;
        }
    }

    // Migrate static files route to routes module TODO
    // "GET /static/*" (static files)
    if (strcmp(req->method, "GET") == 0
            && strncmp(req->uri, "/static/", 8) == 0)
    {
        char * suffix = req->uri + 8;
        int data_len = 7 + strlen(suffix);
        char * data = malloc(data_len + 1);
        sprintf(data, "static/%s", suffix);

        result.data = data;
        result.data_type = ACTION_FILE_PATH;
        result.http_code = 200;
        result.clean_data = true;
    }

    // Cleanup
    clear_url_detail(&req_detail);

    return result;
}
