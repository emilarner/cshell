#include "string.h"

string *string_init()
{
    string *s = (string*) malloc(sizeof(string));

    s->capacity = 256;
    s->length = 0;
    s->data = (char*) malloc(s->capacity);

    memset(s->data, 0, s->capacity);
    
    return s;
}

size_t string_scat(string *str, string *str2);

size_t string_ccat(string *str, char *stuff)
{
    size_t stuff_len = strlen(stuff);

    if ((str->length + stuff_len) >= str->capacity)
    {
        str->capacity *= 2;
        str->data = (char*) realloc(str->data, str->capacity);
    }

    strcat(str->data + str->length, stuff);
    /*               ^^^^^^ this saves resources */

    str->length += stuff_len;
}

void string_free(string *str)
{
    free(str->data);
    free(str);
}