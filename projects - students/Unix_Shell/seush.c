#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/wait.h>

typedef struct link_node {
    void* value;
    struct link_node* next;
} link_node;

typedef struct link_t {
    link_node* head;
    link_node* tail;
    unsigned int length;
} link_t;

link_t link_init() {
    link_t new_link;
    new_link.head = new_link.tail = NULL;
    new_link.length = 0;
    return new_link;
}

void link_add(link_t* l, void* value) {
    link_node* new_node = (link_node *)malloc(sizeof(link_node));
    new_node->value = value;
    if (l->head == NULL)
        l->head = l->tail = new_node;
    else {
        new_node->next = l->tail->next;
        l->tail->next = new_node;
        l->tail = new_node;
    }
    l->length++;
}

void link_clear(link_t* l) {
    link_node* current_node = l->head, *tmp_node;
    while (current_node) {
        tmp_node = current_node->next;
        free(current_node);
        current_node = tmp_node;
    }
    l->head = l->tail = NULL;
    l->length = 0;
}

void parse_cmd(char* cmd_line, char** cmd_name, char*** cmd_argv) {
    link_t arg_link = link_init();
    char* token = strtok(cmd_line, " \t\n\r");
    while (token != NULL) {
        link_add(&arg_link, token);
        token = strtok(NULL, " \t\n\r");
    }

    *cmd_name = arg_link.head->value;
    char** argv = malloc(sizeof(char*) * (arg_link.length));
    link_node* tmp_node = arg_link.head->next; 
    for (unsigned int i = 0; tmp_node; i++) {
        argv[i] = tmp_node->value;
        tmp_node = tmp_node->next;
    }
    argv[arg_link.length - 1] = NULL;
    *cmd_argv = argv;

    link_clear(&arg_link);
}

void exec_cmd(char* cmd_line, size_t cmd_line_length) {
    char* cmd_name, ** cmd_argv;
    parse_cmd(cmd_line, &cmd_name, &cmd_argv);

    pid_t child_pid = fork();
    if (child_pid) {
        wait(NULL);
    } else {
        execv(cmd_name, cmd_argv);
    }
}

int main(char** argv, int argc) {
    while (true) {
        char* cmd_line = NULL;
        size_t cmd_line_length = 0;

        printf("seush> ");
        getline(&cmd_line, &cmd_line_length, stdin);
        exec_cmd(cmd_line, cmd_line_length);
    }

    return 0;
}
