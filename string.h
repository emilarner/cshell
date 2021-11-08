/* Because strings in C are unbearable, let's make our own, featuring dynamic allocation! */

#ifndef STRING_H
#define STRING_H

#include <string.h>
#include <stdlib.h>

struct string
{
    char *data;
    size_t capacity;
    size_t length;
};

typedef struct string string;

string *string_init();

size_t string_length(string *str);
size_t string_scat(string *str, string *str2);
size_t string_ccat(string *str, char *stuff);

void string_free(string *str);



#endif 