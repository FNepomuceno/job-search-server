#include <stdlib.h>
#include <string.h>

#include "vallist.h"
#include "template.h"

char * parse_html_template(char * raw_html, val_map * context)
{
    if (context == NULL) { return raw_html; }

    // Get length and components of new string
    long new_length = 0;
    val_list sections = new_list();
    char * cursor = raw_html;

    while (cursor != NULL)
    {
        char * next_var = strstr(cursor, "{");

        if (next_var == NULL)
        {
            // No more variables
            long section_length = strlen(cursor);
            new_length += section_length;
            list_insert(&sections, cursor, section_length);

            cursor = NULL;
        }
        else
        {
            char * var_end = strstr(next_var, "}");

            // Section before variable
            long prevar_length = next_var - cursor;
            new_length += prevar_length;
            list_insert(&sections, cursor, prevar_length);

            // Section at variable
            long var_length = var_end - (next_var + 1);
            char * template_var = malloc(var_length + 1);
            memcpy(template_var, next_var+1, var_length);
            template_var[var_length] = '\0';

            char * value = map_get(context, template_var);
            if (value == NULL) { value = ""; }
            long val_length = strlen(value);

            new_length += val_length;
            list_insert(&sections, value, val_length);

            // Cleanup
            free(template_var);

            cursor = var_end + 1;
        }
    }

    // Setup new string
    char * result = malloc(new_length + 1);
    char * offset = result;

    for (int i = 0; i < sections.size; ++i)
    {
        long section_length = strlen(sections.values[i]);

        memcpy(offset, sections.values[i], section_length);
        offset += section_length;
    }
    result[new_length] = '\0';

    // Cleanup
    clear_val_list(&sections);
    free(raw_html);

    return result;
}
