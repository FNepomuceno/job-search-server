#include <stdlib.h>
#include <string.h>

#include "vallist.h"

void clear_val_list(val_list * list)
{
    if (list == NULL) return;
    for (int i = 0; i < list->size; ++i)
    {
        free(list->values[i]);
    }
    free(list->values);
    list->size = 0;
}

int list_find(val_list * list, char * value)
{
    for (int i = 0; i < list->size; ++i)
    {
        if (strcmp(list->values[i], value) == 0)
        {
            return i;
        }
    }

    return -1;
}

val_list new_list(void)
{
    val_list result;

    result.capacity = 10;
    result.size = 0;
    result.values = malloc(result.capacity * sizeof (char *));

    return result;
}

void list_insert(val_list * list, char * value, long value_len)
{
    // Increase capacity if needed
    if (list->size >= list->capacity)
    {
        list->capacity *= 2;
        list->values = realloc(list->values,
            list->capacity * sizeof (char *));
    }

    // Insert new value
    list->values[list->size] = malloc(value_len+1);
    memcpy(list->values[list->size], value, value_len);
    list->values[list->size][value_len] = '\0';

    ++list->size;
}
