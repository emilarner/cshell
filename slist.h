/* An implementation of a string list in C. */

#ifndef SLIST_H
#define SLIST_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "string.h"

struct slist
{
    char **data;
    size_t length;
    size_t capacity;
};

typedef struct slist slist;

slist *slist_init(void);
slist *slist_from_charpp(char **data, size_t len);
slist *slist_cat(slist *one, slist *two);
string *slist_join(slist *list, char *separator);

void slist_print(slist *l, char *name);

void slist_push(slist *s, char *string);
void slist_pop(slist *s);
char *slist_get(slist *s, size_t index);
void slist_free(slist *s);

slist *split(char *string, char *delimiter, char escape);

#endif