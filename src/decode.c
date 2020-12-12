#include <stdlib.h>
#include <string.h>

#include "decode.h"

void clear_query_map(query_map * map)
{
    clear_val_map(map);
}

int key_index_query(query_map * map, char * key)
{
    return map_key_index(map, key);
}

query_map decode_query(char * query_string)
{
    query_map result = new_map();

    if (*query_string == '\0') { return result; }
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
