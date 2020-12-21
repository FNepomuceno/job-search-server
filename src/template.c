#include <stdlib.h>
#include <string.h>

#include "template.h"

char * parse_html_template(char * raw_html, val_map * context)
{
    if (context == NULL) { return raw_html; }

    // Calculate length of new char *
    long new_length = strlen(raw_html);

    char * result = malloc(new_length + 1);

    // Cleanup
    free(raw_html);

    return result;
}
