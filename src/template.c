#include <stdlib.h>
#include <string.h>

#include "template.h"

char * parse_html_template(char * raw_html, val_map * context)
{
    if (context == NULL) { return raw_html; }

    // Calculate length of new char *
    long new_length = strlen(raw_html);

    // Setup new string
    char * result = malloc(new_length + 1);

    // Finish actual implementation TODO
    memcpy(result, raw_html, new_length);
    result[new_length] = '\0';

    // Cleanup
    free(raw_html);

    return result;
}
