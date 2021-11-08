#ifndef TEXT_H
#define TEXT_H

#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool strequ(char *one, char *two);
bool memequ(void *one, void *two, size_t len);
bool strin(char *parent, char *subset);
bool startswith(char *starts, char *with);

void *memcpy_s(void *dest, void *src, size_t length, size_t limit);

char *trimwhitespace(char *str);
char *wstrip(char *text);
char *strip(char *text, char *what);


#endif