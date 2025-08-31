#include <stdio.h>
#include <stdlib.h>
//compiler avec gcc -fPIC -shared -o test.so test.c
char* get_msg(char * game_name) {
    char *str = malloc(256);
    if (!str) return NULL;
    sprintf(str, "la lib c fonctionne %s", game_name);
    return str;
}
