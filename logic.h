#ifndef LOGIC_H
#define LOGIC_H

#include "appcontext.h"

typedef struct {
    const char* str;
    Column column;
    YearInfo years;
} AppParams;

void initContext(AppContext* context);
int compareRecords(const void* a, const void* b);
void runLoadDataTask(AppContext* context, const char* fileName);
void runCalculateMetricsTask(AppContext* context, AppParams* params);
void disposeContext(AppContext* context);

#endif // LOGIC_H
