#include <stdlib.h>
#include <string.h>

#include "decode.h"

void clear_query_map(query_map * map)
{
    if (map == NULL) return;
    for (int i = 0; i < map->size; i++)
    {
        free(map->keys[i]);
        free(map->values[i]);
    }
    free(map->keys);
    free(map->values);
}

int key_index_query(query_map * map, char * key)
{
    for (int i = 0; i < map->size; i++)
    {
        if (strcmp(map->keys[i], key) == 0)
        {
            return i;
        }
    }

    return -1;
}

query_map decode_query(char * query_string)
{
    query_map result;

    // Set up dynamically sized array of char *
    result.capacity = 10;
    result.size = 0;
    result.keys = malloc(result.capacity * sizeof (char *));
    result.values = malloc(result.capacity * sizeof (char *));

    if (*query_string == '\0')
    {
        return result;
    }
    else if (*query_string == '?')
    {
        ++query_string;
    }

    // Get key/value pairs
    char * cur_pair = query_string;

    do
    {
        if (result.size >= result.capacity)
        {
            result.capacity *= 2;
            result.keys = realloc(result.keys,
                    result.capacity * sizeof (char *));
            result.values = realloc(result.values,
                    result.capacity * sizeof (char *));
        }

        char * key = cur_pair;
        char * value = strstr(cur_pair, "=") + 1;

        char * separator = strstr(cur_pair, "&");
        if (separator == NULL)
        {
            separator = value + strlen(value);
        }

        long key_len = value - 1 - key;
        char * result_key = malloc(key_len + 1);
        memcpy(result_key, key, key_len);
        result_key[key_len] = '\0';
        result.keys[result.size] = result_key;

        long value_len = separator - value;
        char * result_value = malloc(value_len + 1);
        memcpy(result_value, value, value_len);
        result_value[value_len] = '\0';
        result.values[result.size] = result_value;

        if (*separator == '\0')
        {
            cur_pair = NULL;
        }
        else
        {
            cur_pair = separator + 1;
        }
        result.size += 1;
    }
    while (cur_pair != NULL);

    return result;
}
