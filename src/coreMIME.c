#include <coreMIME.h>
#include <fcntl.h>
#include <coreJson.h>
#include <unistd.h>

Json *MIME;

int MIME_init(const char *file) {
    int fileFd = open(file, O_RDONLY);
    if (fileFd == -1) return 1;

    char readBuf[1024];
    read(fileFd, readBuf, 1024);
    close(fileFd);

    MIME = Json_stringInit(readBuf);
}

char *getFileMIME(char *fileType) {
    // char jsonSearchTmp = 
}

void MIME_free() {
    Json_free(MIME);
}
