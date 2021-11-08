#include "text.h"

void *memcpy_s(void *dest, void *src, size_t length, size_t limit)
{
    if (length > limit)
        return NULL;

    return memcpy(dest, src, length);
}

/* Is this string equal to the other one? */
bool strequ(char *one, char *two)
{
    return !strcmp(one, two);
}

/* Is this block of memory--of length len--equal to the other one? */
bool memequ(void *one, void *two, size_t len)
{
    return !memcmp(one, two, len);
}

/* Is subset a subset of parent? -- does parent contain subset? */ 
bool strin(char *parent, char *subset)
{
    return !!strstr(parent, subset);
}

char *wstrip(char *text)
{
    int i = 0;

    while (isspace(*(text + i)))
        i++;

    return text + i;
}

bool startswith(char *starts, char *with)
{
    return memcmp(starts, with, strlen(with));
}

char *strip(char *text, char *what)
{
    return strstr(text, what) + strlen(what);
}


/* https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way */
/* too lazy to do it myself */
char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}