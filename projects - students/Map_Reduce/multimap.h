#ifndef __multimap_h__
#define __multimap_h__

#include "arraylist.h"
#include <string.h>

typedef unsigned long (*hash_func_t)(char* key, int hash_table_size);

typedef struct multi_map {
    array_list* lists;
    size_t table_size;
    hash_func_t hash_func;
} multi_map;

typedef struct kv_pair {
    char* key, * value;
} kv_pair;

multi_map init_map(hash_func_t hash_func, int hash_table_size);

void put_item(multi_map* map, char* key, char* value);

void destroy_map(multi_map* map);

#endif