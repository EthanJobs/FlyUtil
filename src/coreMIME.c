#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <coreJson.h>
#include <coreMIME.h>

Json *MIME;

int MIME_init() {
    int fileFd = open(MIME_FILE, O_RDONLY);
    if (fileFd == -1) return 1;

    MIME = Json_initByFd(fileFd);
}

char *MIME_getFileMIME(char *fileName) {
    int index;
    for (index = strlen(fileName); index >= 0 && fileName[index] != '.'; index--);
    if (fileName[index] != '.') return NULL;
    jsonValue *jv = Json_getValueInObj(MIME, fileName + index + 1);
    if (jv == NULL || jv->n_dataType != JSONSTR) return "";
    return (char *)(jv->n_data.p_data);
}

void MIME_free() {
    Json_free(MIME);
}
