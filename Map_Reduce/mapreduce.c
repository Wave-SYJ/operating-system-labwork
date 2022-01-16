#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#include "arraylist.h"
#include "mapreduce.h"
#include "multimap.h"

typedef struct stat stat_t;
typedef struct file_stat {
    char* path;
    stat_t stat;
} file_stat;

Mapper mapper_func;
Reducer reducer_func;

sem_t emit_sem;
multi_map pair_map;

unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}

void MR_Emit(char *key, char *value) {
    sem_wait(&emit_sem);
    put_item(&pair_map, strdup(key), strdup(value));
    sem_post(&emit_sem);
}

void* __map_thread_runner(void* param) {
    array_list* file_list = (array_list*) param;
    for (size_t i = 0; i < file_list->length; i++)
        (*mapper_func)(((char**)file_list->array)[i]);
}

int __compare_file_size(const void* file1, const void* file2) {
    return ((file_stat*)file1)->stat.st_size - ((file_stat*)file2)->stat.st_size;
}

void __exec_map(Mapper map, int num_mappers, int file_count, char* files[]) {
    mapper_func = map;
    file_stat* stats = malloc(sizeof(file_stat) * file_count);
    size_t total_size = 0;

    for (int i = 0; i < file_count; i++) {
        stats[i].path = files[i];
        if (stat(files[i], &stats[i].stat)) {
            fprintf(stderr, "cannot open file %s\n", files[i]);
            exit(1);
        }
        total_size += stats[i].stat.st_size;
    }
    qsort(stats, file_count, sizeof(file_stat), __compare_file_size);

    array_list* file_lists = malloc(num_mappers * sizeof(array_list));
    for (int i = 0; i < num_mappers; i++)
        file_lists[i] = init_list();

    int allocated = 0;
    for (int i = 0; i < num_mappers; i++) {
        size_t accum_size = 0;
        while (accum_size < total_size / num_mappers && allocated < file_count) {
            push_item(file_lists + i, stats[allocated].path);
            accum_size += stats[allocated].stat.st_size;
            allocated++;
        }
    }       

    pthread_t* tids = malloc(sizeof(pthread_t) * num_mappers);
    for (int i = 0; i < num_mappers; i++)
        pthread_create(tids + i, NULL, __map_thread_runner, file_lists + i);
    for (int i = 0; i < num_mappers; i++)
        pthread_join(tids[i], NULL);

    for (int i = 0; i < num_mappers; i++)
        destroy_list(file_lists + i);
    free(file_lists);
    free(stats);
}

int __compare_pair(const void* pair1, const void* pair2) {
    const char* str1 = (*((kv_pair**) pair1))->key, * str2 = (*((kv_pair**) pair2))->key;
    int result = strcmp(str1, str2);
    return result;
}

char* __get_next_value(char *key, int partition_number) {
    array_list* list = &pair_map.lists[partition_number];
    if (list->iter_pos >= list->length)
        return NULL;
    kv_pair* current = (kv_pair *)list->array[list->iter_pos];
    if (strcmp(current->key, key))
        return NULL;
    list->iter_pos++;
    return current->value;
}

void* __reduce_thread_runner(void* param) {
    size_t index = (size_t) param;
    array_list* list = &pair_map.lists[index];
    qsort(list->array, list->length, sizeof(void*), __compare_pair);
    for (size_t i = 0; i < list->length; i++) {
        if (!i || strcmp(((kv_pair*)list->array[i - 1])->key, ((kv_pair*)list->array[i])->key))
            reducer_func(((kv_pair*)list->array[i])->key, __get_next_value, index);
    }
}

void __exec_reduce(Reducer reduce, int num_reducers) {
    reducer_func = reduce;
    pthread_t* tids = malloc(sizeof(pthread_t) * num_reducers);
    for (size_t i = 0; i < num_reducers; i++)
        pthread_create(tids + i, NULL, __reduce_thread_runner, (void *)i);
    for (size_t i = 0; i < num_reducers; i++)
        pthread_join(tids[i], NULL);
}

void MR_Run(int argc, char *argv[], 
	    Mapper map, int num_mappers, 
	    Reducer reduce, int num_reducers, 
	    Partitioner partition) {

    pair_map = init_map(partition, num_reducers);
    sem_init(&emit_sem, 0, 1);

    __exec_map(map, num_mappers, argc - 1, argv + 1);
    __exec_reduce(reduce, num_reducers);

    sem_destroy(&emit_sem);
    destroy_map(&pair_map);
}