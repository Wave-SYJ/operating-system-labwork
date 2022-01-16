#include "multimap.h"

multi_map init_map(hash_func_t hash_func, int hash_table_size) {
  multi_map map;
  map.lists = malloc(sizeof(array_list) * hash_table_size);
  for (int i = 0; i < hash_table_size; i++)
    map.lists[i] = init_list();
  map.table_size = hash_table_size;
  map.hash_func = hash_func;
  return map;
}

void put_item(multi_map* map, char* key, char* value) {
  size_t hash = (*map->hash_func)(key, map->table_size);
  kv_pair* pair = (kv_pair*)malloc(sizeof(kv_pair));
  pair->key = key;
  pair->value = value;
  push_item(&(map->lists[hash]), pair);
}

void destroy_map(multi_map* map) {
  for (int i = 0; i < map->table_size; i++) {
    for (int j = 0; j < map->lists[i].length; j++)
      free(map->lists[i].array[j]);
    destroy_list(&(map->lists[i]));
  }
  free(map->lists);
  map->table_size = 0;
  map->hash_func = NULL;
}

array_list* get_list(multi_map* map, char* key) {
  size_t hash = (*map->hash_func)(key, map->table_size);
  return &map->lists[hash];
}