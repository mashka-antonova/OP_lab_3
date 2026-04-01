#ifndef ENTRYPOINT_H
#define ENTRYPOINT_H

#include "appcontext.h"
#include "demography.h"

typedef enum {
    LOAD_DATA,
    CALCULATE_METRICS,
    INITIALIZATION,
    DISPOSE_CONTEXT,
} Operation;

typedef struct {
    const char* str;
    Column column;
    YearInfo years;
} AppParams;

void doOperation(Operation operation, AppContext* context, AppParams* params);

#endif // ENTRYPOINT_H
