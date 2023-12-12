#include <coreCookie.h>
#include <coreMd5.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *Cookie_get(char *name, char *password_md5, char *lastDate) {
    int len = strlen(name) + strlen(password_md5) + strlen(lastDate) + strlen(COOKIE_KEY);
    if (len <= 0) return NULL;
    char *Str = (char *)malloc(sizeof(char) * len);
    Str[0] = '\0';
    strcat(Str, name);
    strcat(Str, password_md5);
    strcat(Str, lastDate);
    strcat(Str, COOKIE_KEY);
    char *retStr = Md5_get(Str, strlen(Str));
    free(Str);

    return retStr;
}
