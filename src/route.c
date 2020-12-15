#include "route.h"
#include "valmap.h"

web_action web_index(val_map * params)
{
    web_action result;
    result.redirect_uri = "";
    result.clean_data = false;

    result.data = "static/html/index.html";
    result.data_type = ACTION_FILE_PATH;
    result.http_code = 200;

    return result;
}
