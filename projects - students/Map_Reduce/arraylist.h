#ifndef __arraylist_h__
#define __arraylist_h__

#include <stdlib.h>
#include <string.h>

typedef struct array_list {
    void** array;
    size_t length, __volume;
    size_t iter_pos;
} array_list;

array_list init_list();

void push_item(array_list* list, void* item);

void destroy_list(array_list* list);

#endif