#ifndef CSHELL_H
#define CSHELL_H

#define _GNU_SOURCE

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>

#include <glob.h>
#include <dirent.h>

#include <unistd.h>
#include <sys/wait.h>

#include <glib.h>

#include "text.h"
#include "string.h"
#include "slist.h"


/* Macro is required for deleting allocated memory and generally making life easier. */
#define die(msg) do {       \
    fprintf(stderr, msg);   \
    slist_free(words);      \
    return;                 \
 } while (0)                \


#define peace() do {        \
    slist_free(words);      \
    return;                 \
} while(0)                  \


struct interpreter
{
    char **envp;
    size_t envp_len; 

    size_t lineno;

    GHashTable *variables;
    GHashTable *functions;
    GHashTable *aliases;

    slist *temp_envs;
    slist *temp_envs_prev;

    bool async; 

    bool if_on;
    bool if_condition;
    bool else_on;

    char *function;

    int status;
};


slist *resolve_variables(struct interpreter *i, char *source);

void set_var(struct interpreter *i, char *key, char *value);
char *get_var(struct interpreter *i, char *key);

struct interpreter *interpreter_init(char **envp);
void interpreter_free(struct interpreter *i);

void parse_line(struct interpreter *i, char *line, bool segment);
bool test_expressions(struct interpreter *in, char *line);

#endif