#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <coreJson.h>

int main() {
    int fileFd = open("./json.json", O_RDONLY);
    char buf[1024];
    int readLen = 0;
    // read(fileFd, buf, 1024);
    Json *j = Json_initByFd(fileFd);
    Json_displayValue(j);
    read(fileFd, buf, 1024);
    /* jsonValue *jv = Json_getValue(j, "{\"api\"}");
    jsonValue_print(jv);
    printf("%d", jv->n_dataType);
    */
    Json_free(j);
    return 0;
}