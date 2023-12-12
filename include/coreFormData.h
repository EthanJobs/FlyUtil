#ifndef _COREFORMDATA_H
#define _COREFORMDATA_H

#include <coreTree.h>

#define FORMDATASTR 0
#define FORMDATAFILE 1

typedef struct FormDataValue {
    int n_dataType;
    void *n_data;
} FormDataValue;

typedef Tree FormData;

#endif