#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include "list.h"
#include "metrix.h"

typedef enum {
    OK,
    ERR_FILE_OPEN,
    ERR_INVALID_HEADER,
    ERR_MALLOC_FAILED,
    ERR_EMPTY_DATA,
    ERR_INVALID_REGION,
    ERR_INVALID_COLUMN,
} Status;

typedef struct RowsInfo {
    int totalRows;
    int invalidRows;
} RowsInfo;

typedef struct AppContext {
    LinkedList* list;
    RowsInfo rowsInfo;
    Status programmStatus;
    Metrix metrix;
    LinkedList* graphPoints;
} AppContext;

#endif // APPCONTEXT_H
