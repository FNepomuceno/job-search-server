#ifndef DECODE_H
#define DECODE_H

typedef struct query_map {
    int capacity;
    int size;
    char ** keys;
    char ** values;
} query_map;

void clear_query_map(query_map * map);
int key_index(query_map * map, char * key);
query_map decode_query(char * query_string);

#endif
