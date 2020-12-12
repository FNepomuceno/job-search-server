#ifndef DECODE_H
#define DECODE_H

#include "valmap.h"

typedef val_map query_map;

void clear_query_map(query_map * map);
int key_index_query(query_map * map, char * key);
query_map decode_query(char * query_string);

#endif
