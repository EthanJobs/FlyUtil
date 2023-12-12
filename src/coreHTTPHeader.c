#include <coreHTTPHeader.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

HTTPHeaderLine *HTTPHeaderLine_init(char *method, char *url, char *version);
void HTTPHeaderLine_free(void *a);

HTTPHeaderValue *HTTPHeaderValue_init(char *header, char *value);
void HTTPHeaderValue_free(void *a);
int compareHhvToHhv(void *a, void *b);
int compareStringToHhv(void *a, void *b);

HTTPHeader *HTTPHeader_init(char *method, char *url, char *version);

HTTPHeaderLine *HTTPHeaderLine_init(char *method, char *url, char *version) {
    HTTPHeaderLine *hhl = (HTTPHeaderLine *)malloc(sizeof(HTTPHeaderLine));
    hhl->n_method = method;
    hhl->n_url = url;
    hhl->n_version = version;

    return hhl;
}

void HTTPHeaderLine_free(void *a) {
    if (a == NULL) return;

    HTTPHeaderLine *hhl = (HTTPHeaderLine *)a;
    if (hhl->n_method) free(hhl->n_method);
    if (hhl->n_url) free(hhl->n_url);
    if (hhl->n_version) free(hhl->n_version);

    return;
}

HTTPHeaderValue *HTTPHeaderValue_init(char *header, char *value) {
    HTTPHeaderValue *hhv = (HTTPHeaderValue *)malloc(sizeof(HTTPHeaderValue));
    hhv->n_header = header;
    hhv->n_value = value;

    return hhv;
}

void HTTPHeaderValue_free(void *a) {
    if (a == NULL) return;

    HTTPHeaderValue *hhv = (HTTPHeaderValue *)a;
    if (hhv->n_header) free(hhv->n_header);
    if (hhv->n_value) free(hhv->n_value);

    return;
}

int compareHhvToHhv(void *a, void *b) {
    return strcmp(((HTTPHeaderValue *)a)->n_header, ((HTTPHeaderValue *)b)->n_header);
}

int compareStringToHhv(void *a, void *b) {
    return strcmp((char *)a, ((HTTPHeaderValue *)b)->n_header);
}

HTTPHeader *HTTPHeader_init(char *method, char *url, char *version) {
    HTTPHeader *hh = (HTTPHeader *)malloc(sizeof(HTTPHeader));
    hh->n_HTTPHeaderLine = HTTPHeaderLine_init(method, url, version);
    hh->n_HTTPHeaderValues = Tree_init(compareHhvToHhv, HTTPHeaderValue_free);

    return hh;
}

HTTPHeader *HTTPHeader_fd_init(int fd) {
    char tmp, buf[1024];
    char *method, *url, *version;
    int index, isFree;

    for (index = 0, isFree = read(fd, &tmp, 1); tmp != ' '; index++, isFree = read(fd, &tmp, 1)) {
        if (tmp == EOF || isFree <= 0 || tmp == '\r') return NULL;
        buf[index] = tmp;
    }
    buf[index++] = '\0';
    method = (char *)malloc(sizeof(char) * index);
    strncpy(method, buf, index);

    for (index = 0, isFree = read(fd, &tmp, 1); tmp != ' '; index++, isFree = read(fd, &tmp, 1)) {
        if (tmp == EOF || isFree <= 0 || tmp == '\r') {
            free(method);
            return NULL;
        }
        buf[index] = tmp;
    }
    buf[index++] = '\0';
    url = (char *)malloc(sizeof(char) * index);
    strncpy(url, buf, index);

    for (index = 0, isFree = read(fd, &tmp, 1); tmp != '\r'; index++, isFree = read(fd, &tmp, 1)) {
        if (tmp == EOF || isFree <= 0) {
            free(url);
            free(method);
            return NULL;
        }
        buf[index] = tmp;
    }

    isFree = read(fd, &tmp, 1);
    if (isFree <= 0 || tmp != '\n') {
        free(url);
        free(method);
        return NULL;
    }

    buf[index++] = '\0';
    version = (char *)malloc(sizeof(char) * index);
    strncpy(version, buf, index);

    HTTPHeader *hh = HTTPHeader_init(method, url, version);

    while (1) {
        index = 0;
        int judge = 0; // 空格判断
        for (isFree = read(fd, &tmp, 1); isFree > 0 && tmp != '\r' && tmp != ':' && tmp != EOF; isFree = read(fd, &tmp, 1)) {
            if (tmp != ' ') judge = 1;
            if (judge) buf[index++] = tmp;
        }
        if (isFree <= 0 || tmp == EOF) break;
        if (tmp == '\r') {
            isFree = read(fd, &tmp, 1);
            if (isFree <= 0 || tmp != '\n') break;
            else return hh;
        }
        buf[index++] = '\0';
        char *header = (char *)malloc(sizeof(char) * index);
        strncpy(header, buf, index);

        index = 0;
        judge = 0;
        for (isFree = read(fd, &tmp, 1); isFree > 0 && tmp != '\r' && tmp != EOF; isFree = read(fd, &tmp, 1)) {
            if (tmp != ' ') judge = 1;
            if (judge) buf[index++] = tmp;
        }
        if (isFree <= 0 || tmp == EOF) break;
        isFree = read(fd, &tmp, 1);
        if (isFree <= 0 || tmp != '\n') break;
        buf[index++] = '\0';
        char *value = (char *)malloc(sizeof(char) * index);
        strncpy(value, buf, index);

        HTTPHeader_insertHeaderValue(hh, header, value);
    }

    HTTPHeader_free(hh);
    return NULL;
}

int HTTPHeader_insertHeaderValue(HTTPHeader *hh, char *header, char *value) {
    if (hh == NULL || header == NULL || value == NULL) return -1;

    return Tree_insertValue(hh->n_HTTPHeaderValues, HTTPHeaderValue_init(header, value));
}

char *HTTPHeader_getHeaderValue(HTTPHeader *hh, char *header) {
    if (hh == NULL || header == NULL) return NULL;

    HTTPHeaderValue *hhv = (HTTPHeaderValue *)Tree_getValue(hh->n_HTTPHeaderValues, (void *)header, compareStringToHhv);
    if (hhv == NULL) return NULL;
    return hhv->n_value;
}

void HTTPHeaderValue_fd_print(HTTPHeaderValue *hhv, int fd) {
    if (hhv == NULL || fd < 0) return;

    dprintf(fd, "%s: %s\r\n", hhv->n_header, hhv->n_value);

    return;
}

void HTTPHeader_fd_print(HTTPHeader *hh, int fd) {
    if (hh == NULL) return;

    if (hh->n_HTTPHeaderLine != NULL) {
        if (hh->n_HTTPHeaderLine->n_method != NULL) dprintf(fd, "%s", hh->n_HTTPHeaderLine->n_method);
        if (hh->n_HTTPHeaderLine->n_url != NULL) dprintf(fd, " %s", hh->n_HTTPHeaderLine->n_url);
        if (hh->n_HTTPHeaderLine->n_version != NULL) dprintf(fd, " %s\r\n\r\n", hh->n_HTTPHeaderLine->n_version);
    }

    Iterator *i = Tree_getInorderIterator(hh->n_HTTPHeaderValues);
    while (i->fun_hasNext(i))
        HTTPHeaderValue_fd_print(i->fun_next(i), fd);
    Iterator_free(i);
    dprintf(fd, "\r\n");

    return;
}

void HTTPHeader_free(HTTPHeader *hh) {
    if (hh == NULL) return;

    HTTPHeaderLine_free(hh->n_HTTPHeaderLine);
    Tree_free(hh->n_HTTPHeaderValues);
    free(hh);

    return;
}