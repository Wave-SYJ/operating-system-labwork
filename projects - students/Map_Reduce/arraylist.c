#include "arraylist.h"

const int init_volume = 8;

array_list init_list(size_t item_size) {
    array_list list;
    list.length = 0;
    list.__volume = init_volume;
    list.array = malloc(init_volume * sizeof(void*));
    return list;
}

void __ensure_volume(array_list* list, size_t required_volume) {
    if (list->__volume >= required_volume)
        return;

    size_t target = list->__volume;
    while (target < required_volume)
        target *= 2;

    void** new_array = malloc(target * sizeof(void*));
    memcpy(new_array, list->array, list->length * list->__volume);
    free(list->array);

    list->array = new_array;
    list->__volume = target;
}

void push_item(array_list* list, void* item) {
    __ensure_volume(list, list->length + 1);
    list->array[list->length] = item;
    list->length++;
}

void destroy_list(array_list* list) {
    if (list->array)
        free(list->array);
    list->length = 0;
    list->array = NULL;
}
