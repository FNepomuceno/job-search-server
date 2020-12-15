#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "query.h"

void clear_url_detail(url_detail * detail)
{
    clear_val_list(&detail->url);
    clear_val_map(&detail->query);
}

url_detail req_to_detail(char * method, char * url)
{
    url_detail result;

    result.url = new_list();
    result.method = method;

    // Insert sections
    char * start = strstr(url, "/");
    char * end = strstr(start+1, "/");
    while (end != NULL)
    {
        list_insert(&result.url, start+1, end-start-1);

        // Go to next section
        start = end;
        end = strstr(end+1, "/");
    }

    if (start != NULL)
    {
        end = strstr(start, "?");
        if (end == NULL) { end = start + strlen(start); }

        list_insert(&result.url, start+1, end-start-1);
    }

    // Insert queries
    result.query = query_to_map(strstr(url, "?"));

    return result;
}

val_map query_to_map(char * query_string)
{
    val_map result = new_map();

    if (query_string == NULL || *query_string == '\0') { return result; }
    else if (*query_string == '?') { ++query_string; }

    char * cur_pair = query_string;
    while (true)
    {
        char * key = cur_pair;
        char * value = strstr(cur_pair, "=") + 1;

        char * separator = strstr(cur_pair, "&");
        if (separator == NULL)
        {
            separator = value + strlen(value);
        }

        long key_len = value - 1 - key;
        long value_len = separator - value;

        insert_entry(&result, key, key_len, value, value_len);

        if (*separator == '\0') { break; }
        cur_pair = separator + 1;
    }

    return result;
}
