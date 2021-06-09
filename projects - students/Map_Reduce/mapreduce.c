#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#include "arraylist.h"
#include "mapreduce.h"

typedef struct kv_pair {
    const char* key, * value;
} kv_pair;

typedef struct stat stat_t;

array_list pair_list;
Mapper mapper_func;
Reducer reducer_func;

sem_t emit_sem;

unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}

void MR_Emit(char *key, char *value) {
    kv_pair* new_pair = malloc(sizeof(kv_pair));
    new_pair->key = key;
    new_pair->value = value;

    sem_wait(&emit_sem);
    push_item(&pair_list, new_pair);
    sem_post(&emit_sem);
}

void* __map_thread_runner(void* param) {
    array_list* file_list = (array_list*) param;
    for (size_t i = 0; i < file_list->length; i++)
        mapper_func(file_list->array[i]);
}

int __compare_file_size(const void* file1, const void* file2) {
    return ((stat_t*)file1)->st_size - ((stat_t*)file2)->st_size;
}

void __exec_map(Mapper map, int num_mappers, int file_count, char* files[]) {
    mapper_func = map;
    stat_t* stats = malloc(sizeof(stat_t) * file_count);
    for (int i = 0; i < file_count; i++) {
        if (stat(files[i], &stats[i])) {
            fprintf(stderr, "cannot open file %s\n", files[i]);
            exit(1);
        }
    }
    qsort(stats, file_count, sizeof(stat_t), __compare_file_size);

    array_list* file_lists = malloc(num_mappers * sizeof(array_list));
    for (int i = 0; i < num_mappers; i++)
        file_lists[i] = init_list();

    int allocated = 0;
    while (allocated < file_count)
        for (int i = 0; i < file_count && allocated < file_count; i++, allocated++)
            push_item(file_lists + i, files[allocated]);

    pthread_t* tids = malloc(sizeof(pthread_t) * num_mappers);
    for (int i = 0; i < num_mappers; i++)
        pthread_create(tids + i, NULL, __map_thread_runner, file_lists + i);
    for (int i = 0; i < num_mappers; i++)
        pthread_join(tids[i], NULL);

    for (int i = 0; i < file_count; i++)
        destroy_list(file_lists + i);
    free(file_lists);
    free(stats);
}

void MR_Run(int argc, char *argv[], 
	    Mapper map, int num_mappers, 
	    Reducer reduce, int num_reducers, 
	    Partitioner partition) {

    pair_list = init_list();
    sem_init(&emit_sem, 0, 1);

    __exec_map(map, num_mappers, argc - 1, argv + 1);

    kv_pair** pair_array = (kv_pair**)pair_list.array;
    for (size_t i = 0; i < pair_list.length; i++) {
        printf("%s %s\n", pair_array[i]->key, pair_array[i]->value);
    }

    sem_destroy(&emit_sem);
    destroy_list(&pair_list);
}