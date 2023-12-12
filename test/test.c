#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <coreJson.h>

int main() {
    int fileFd = open("./test/json.json", O_RDONLY);
    char buf[1024];
    int readLen = 0;
    read(fileFd, buf, 1024);
    Json *j = Json_stringInit(buf);
    Json_displayValue(j);
    /* jsonValue *jv = Json_getValue(j, "{\"api\"}");
    jsonValue_print(jv);
    printf("%d", jv->n_dataType);
 */
    return 0;
}