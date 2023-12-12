#ifndef _COREMIME_H
#define _COREMIME_H

int MIME_init(const char *file);
char *getFileMIME(char *fileType);
void MIME_free();

#endif