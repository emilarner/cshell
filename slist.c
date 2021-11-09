#include "slist.h"

/* Initialize the string list structure */
slist *slist_init(void)
{
    slist *s = (slist*) malloc(sizeof(slist));
    
    s->length = 0;
    s->capacity = 12;
    s->data = (char**) malloc(s->capacity * sizeof(char*));
    /*                   ^^^ let it be known that I accidentally wrote 'sizeof(s->capacity)' instead. */
    /* In otherwords, I corrupted the heap for a while.... */

    return s; 
}

/* Make a list from a char**. (Warning: data is not allocated on the heap!) */
slist *slist_from_charpp(char **data, size_t len)
{
    slist *s = slist_init();

    free(s->data);

    s->data = data;
    s->length = len;
}

/* Concatenate--add together--two lists, returning a list containing the sum of both of them. */
slist *slist_cat(slist *one, slist *two)
{
    slist *result = slist_init();
    size_t length = one->length + two->length;

    free(result->data);

    result->data = (char**) malloc(length * sizeof(char*));

    for (size_t i = 0; i < one->length; i++)
        slist_push(result, one->data[i]);

    for (size_t i = 0; i < two->length; i++)
        slist_push(result, two->data[i]);

    return result;
}

void slist_print(slist *l, char *name)
{
    printf("=== %s ===\n", name);

    for (int i = 0; i < l->length; i++)
        printf("[%d]: %s\n", i, slist_get(l, i));

    printf("=== END ===\n");
}

/* Convert a list to a string, given a joining separator--the last element is without a separator. */
string *slist_join(slist *list, char *separator)
{
    string *result = string_init();

    

    for (int i = 0; i < list->length - 1; i++) {
        string_ccat(result, slist_get(list, i));
        string_ccat(result, separator);
    }

    string_ccat(result, slist_get(list, list->length - 1));
    return result;
}

/* Push an item onto it */
void slist_push(slist *s, char *string)
{
    if (s->length == s->capacity)
    {
        s->capacity *= 2;
        s->data = (char**) realloc(s->data, s->capacity * sizeof(char*));
    }

    s->data[s->length] = string == NULL ? NULL : strdup(string);
    s->length++; 
}


/* Pop--or remove--an item from it. */
void slist_pop(slist *s)
{
    free(s->data[s->length]);
    s->length--;
}

/* Obtain an item from the list. */
char *slist_get(slist *s, size_t index)
{
    if (index > s->length || index < 0)
        return NULL;

    return s->data[index];
}

/* Free the resources used by the list. */
void slist_free(slist *s)
{
    for (size_t i = 0; i < s->length; i++)
    {
        free(s->data[i]);
    }

    free(s->data);
    free(s);
}

/* Split a string by a delimiter, additionally having an escape character. */
slist *split(char *string, char *delimiter, char escape)
{
    slist *s = slist_init();

    char hstring[strlen(string)];
    strcpy(hstring, string);

    /* The bounds of this array must be checked! */ 
    char escape_buffer[1024];
    char *token = strtok(hstring, delimiter);
    bool escaped = false;

    while (token != NULL)
    {
        if (token[0] == escape)
        {
            escaped = true;
            memset(escape_buffer, 0, sizeof(escape_buffer));
            token++; // Skip escape character
        }

        /* Definitely a cleaner way. */
        if (escaped)
        {
            if (token[strlen(token) - 1] == escape)
            {
                char last[256];
                strncpy(last, token, sizeof(last));

                last[strlen(last) - 1] = '\0';
                strcat(escape_buffer, last);

                slist_push(s, escape_buffer);

                escaped = false;

                token = strtok(NULL, delimiter);
                continue;
            }

            strcat(escape_buffer, token);
            strcat(escape_buffer, " ");

            token = strtok(NULL, delimiter);

            continue;
        }

        slist_push(s, token);
        token = strtok(NULL, delimiter);
    }


    return s;
}
