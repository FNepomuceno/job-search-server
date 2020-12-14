#ifndef VALLIST_H
#define VALLIST_H

typedef struct val_list {
    int capacity;
    int size;
    char ** values;
} val_list;

void clear_val_list(val_list * list);
int list_find(val_list * list, char * value);

val_list new_list(void);
void list_insert(val_list * list, char * value, long value_len);

#endif
