#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>

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

void path_add(link_t* path, const char* str) {
    size_t str_len = strlen(str);
    if (str[str_len - 1] == '/')
        str_len--;
    char* new_path = malloc(sizeof(char) * (str_len + 1));
    strncpy(new_path, str, str_len + 1);
    link_add(path, new_path);
}

void path_clear(link_t* path) {
    link_node* current_node = path->head;
    while (current_node) {
        if (current_node->value)
            free(current_node->value);
        current_node = current_node->next;
    }
    link_clear(path);
}

void parse_cmd(char* cmd_line, char** cmd_name, char*** cmd_argv, unsigned int* cmd_argc) {
    link_t arg_link = link_init();
    char* token = '\0'; 
    while ((token = strsep(&cmd_line, " \t\n\r")) != NULL) {
        if (*token != '\0')
            link_add(&arg_link, token);
    }

    if (arg_link.length > 0) {
        *cmd_name = arg_link.head->value;
        char** argv = malloc(sizeof(char*) * (arg_link.length + 1));
        link_node* tmp_node = arg_link.head; 
        for (unsigned int i = 0; tmp_node; i++) {
            argv[i] = tmp_node->value;
            tmp_node = tmp_node->next;
        }
        argv[arg_link.length] = NULL;
        *cmd_argv = argv;
        *cmd_argc = arg_link.length;
    } else {
        *cmd_name = NULL;
        *cmd_argv = NULL; 
        cmd_argc = 0;
    }

    link_clear(&arg_link);
}

typedef enum exec_result {
    SUCCESS,
    ERROR,
    NOT_EXEC
} exec_result;

exec_result exec_built_in_cmd(char* cmd_name, char** cmd_argv, int cmd_argc, link_t* path_link) {
    if (strcmp(cmd_name, "exit") == 0) {
        if (cmd_argc > 1)
            return ERROR;
        exit(0);
    } else if (strcmp(cmd_name, "cd") == 0) {
        if (chdir(cmd_argv[1]) == -1)
            return ERROR;
        return SUCCESS;
    } else if (strcmp(cmd_name, "path") == 0) {
        path_clear(path_link);
        for (int i = 1; cmd_argv[i]; i++)
            path_add(path_link, cmd_argv[i]);
        return SUCCESS;
    }

    return NOT_EXEC;
}

char* trim(char* str) {
    if (str == NULL)
        return str;

    while (isspace(*str)) str++;
    int len = strlen(str);
    while (isspace(str[len - 1])) len--;
    str[len] = 0;
    return str;
}

char* parse_redirect_dest(char* cmd_line, bool* format_error) {
    cmd_line = trim(cmd_line);
    if (cmd_line && cmd_line[0] == '>') {
        *format_error = true;
        return NULL;
    }

    char* divider = strchr(cmd_line, '>');
    if (!divider) {
        *format_error = false;
        return NULL;
    }

    *divider = 0;
    if (strchr(divider + 1, '>')) {
        *format_error = true;
        return NULL;
    }

    char* redirect_dest = trim(divider + 1);
    if (!redirect_dest || !redirect_dest[0]) {
        *format_error = true;
        return NULL;
    }


    for (char* ch = redirect_dest; *ch; ch++)
        if (isspace(*ch)) {
            *format_error = true;
            return NULL;
        }
    
    *format_error = false;
    return redirect_dest;
}

void print_error() {
    static const char* error_msg = "An error has occurred\n";
    write(STDERR_FILENO, error_msg, strlen(error_msg));
}

unsigned int exec_single_cmd(char* cmd_line, link_t* path_link) {
    bool format_error;
    char* redirect_dest = parse_redirect_dest(cmd_line, &format_error);
    if (format_error) {
        print_error();
        return 0;
    }

    char* cmd_name, ** cmd_argv;
    unsigned int cmd_argc;
    parse_cmd(cmd_line, &cmd_name, &cmd_argv, &cmd_argc);
    if (cmd_name == NULL)
        return 0;

    exec_result built_in_exec_result = exec_built_in_cmd(cmd_name, cmd_argv, cmd_argc, path_link); 
    if (built_in_exec_result == ERROR) {
        print_error();
        return 0;
    }
    else if (built_in_exec_result == SUCCESS)
        return 0;
    
    pid_t child_pid = fork();
    if (child_pid)
        return 1;
    else {
        link_node* current_node = path_link->head;
        char* cmd_path = cmd_name;
        bool searching = true;
        while ((searching = access(cmd_path, X_OK)) != 0 && current_node) {
            size_t real_path_size = sizeof(char) *  (strlen(current_node->value) + 1 + strlen(cmd_line) + 1);
            char* real_path = malloc(real_path_size); 
            memset(real_path, 0, real_path_size);

            strcat(real_path, current_node->value);
            strcat(real_path, "/");
            strcat(real_path, cmd_name);

            cmd_path = real_path;
            current_node = current_node->next;
        }

        if (redirect_dest) {
            freopen(redirect_dest, "w", stdout);
            freopen(redirect_dest, "w", stderr);
        }

        if (searching || execv(cmd_path, cmd_argv) == -1) {
            print_error();
            exit(0);          
        }
    }
    return 0;
}

void exec_parallel_cmd(char* cmd_line, link_t* path_link) {
    char* token = '\0';
    unsigned int process_count = 0;

    while ((token = strsep(&cmd_line, "&")) != NULL)
        if (*token != '\0')
            process_count += exec_single_cmd(token, path_link);

    for (unsigned int i = 0; i < process_count; i++)
        wait(NULL);
}

int main(int argc, char** argv) {
    if (argc > 2) {
        print_error();
        exit(1);
    }

    FILE* file = stdin;
    if (argc == 2 && (file = freopen(argv[1], "r", stdin)) == NULL)  {
        print_error();
        exit(1);
    }

    link_t path_link = link_init();
    path_add(&path_link, "/bin");

    while (true) {
        char* cmd_line = NULL;
        size_t cmd_line_length = 0;
        if (argc == 1)
            printf("seush> ");

        if(getline(&cmd_line, &cmd_line_length, stdin) == -1)
            exit(0);
        cmd_line_length = strlen(cmd_line);
        if (isspace(cmd_line[cmd_line_length - 1]))
            cmd_line[cmd_line_length - 1] = '\0';

        exec_parallel_cmd(cmd_line, &path_link);
        free(cmd_line);
    }

    return 0;
}
