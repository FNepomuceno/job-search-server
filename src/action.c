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
    if (action->context != NULL)
    {
        clear_val_map(action->context);
        free(action->context);
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

    for (int i = 0; i < temp_detail.url.size; ++i)
    {
        char * temp_sec = temp_detail.url.values[i];
        char * req_sec = req_detail->url.values[i];
        if (temp_sec[0] == '{')
        {
            // Set given variable to request's section value
            insert_entry(result, temp_sec+1, strlen(temp_sec)-2,
                req_sec, strlen(req_sec));
        }
        else if (strcmp(temp_sec, "*") == 0)
        {
            // Compute sub path's length
            long length = 0;
            for (int j = i; j < req_detail->url.size; ++j)
            {
                length += strlen(req_detail->url.values[j]) + 1;
            }

            // Get the sub path
            char * sub_path = malloc(length+1);
            char * offset = sub_path;
            for (int j = i; j < req_detail->url.size; ++j)
            {
                char * cur_sec = req_detail->url.values[j];
                sprintf(offset, "%s/", cur_sec);
                offset += strlen(req_detail->url.values[j]) + 1;
            }
            if (length > 0) { --length; }
            sub_path[length] = '\0';

            // Set * variable to sub path
            insert_entry(result, "*", 1, sub_path, length);

            // Cleanup
            free(sub_path);

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
    // Malformed request
    if (!req->is_successful) {
        return web_invalid(NULL, NULL, NULL);
    }

    url_detail req_detail = req_to_detail(req->method, req->uri);

    web_route routes[] = {
        { "GET", "/favicon.ico", web_favicon },
        { "GET", "/", web_index },
        { "GET",  "/static/*", web_static },
        { "GET", "/jobs/{job_id}", web_jobs_detail },
        { "GET", "/api/jobs", web_jobs_get },
        { "POST", "/api/jobs", web_jobs_post }
    };

    long num_routes = sizeof (routes) / sizeof (web_route);
    val_map * match = NULL;
    for (int i = 0; i < num_routes; ++i)
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

    // Cleanup
    clear_url_detail(&req_detail);

    return web_not_found(NULL, NULL, NULL);
}
