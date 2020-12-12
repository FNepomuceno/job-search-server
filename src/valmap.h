#ifndef VALMAP_H
#define VALMAP_H

typedef struct val_map {
    int capacity;
    int size;
    char ** keys;
    char ** values;
} val_map;

void clear_val_map(val_map * map);
int map_key_index(val_map * map, char * key);

val_map new_map(void);
void insert_entry(val_map * map, char * key, long key_len, char * value,
    long value_len);

#endif
