#!/usr/bin/bash
gcc -g -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -lglib-2.0 -lreadline -o cshell main.c cshell.c slist.c text.c string.c