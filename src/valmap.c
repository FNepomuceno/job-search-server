#include <stdlib.h>
#include <string.h>

#include "valmap.h"

void clear_val_map(val_map * map)
{
    if (map == NULL) return;
    for (int i = 0; i < map->size; ++i)
    {
        free(map->keys[i]);
        free(map->values[i]);
    }
    free(map->keys);
    free(map->values);
    map->size = 0;
}

int map_key_index(val_map * map, char * key)
{
    for (int i = 0; i < map->size; ++i)
    {
        if (strcmp(map->keys[i], key) == 0)
        {
            return i;
        }
    }

    return -1;
}

int map_find_key(val_map * map, char * key)
{
    return map_key_index(map, key);
}

char * map_get(val_map * map, char * key)
{
    int index = map_find_key(map, key);

    if (index < 0) { return NULL; }
    else { return map->values[index]; }
}

bool map_has(val_map * map, char * key)
{
    return map_find_key(map, key) >= 0;
}

val_map new_map(void)
{
    val_map result;

    result.capacity = 10;
    result.size = 0;
    result.keys = malloc(result.capacity * sizeof (char *));
    result.values = malloc(result.capacity * sizeof (char *));

    return result;
}

void insert_entry(val_map * map, char * key, long key_len, char * value,
    long value_len)
{
    int key_index = map_key_index(map, key);

    // Update value if key exists
    if (key_index >= 0)
    {
        map->values[key_index] = realloc(map->values[key_index],
            strlen(value) + 1);
        strcpy(map->values[key_index], value);
        return;
    }

    // Increase capacity if needed
    if (map->size >= map->capacity)
    {
        map->capacity *= 2;
        map->keys = realloc(map->keys, map->capacity * sizeof (char *));
        map->values = realloc(map->values,
            map->capacity * sizeof (char *));
    }

    // Insert new key value pair
    map->keys[map->size] = malloc(key_len+1);
    memcpy(map->keys[map->size], key, key_len);
    map->keys[map->size][key_len] = '\0';

    map->values[map->size] = malloc(value_len+1);
    memcpy(map->values[map->size], value, value_len);
    map->values[map->size][value_len] = '\0';

    ++map->size;
}
