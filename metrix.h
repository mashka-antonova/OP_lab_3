#ifndef METRIX_H
#define METRIX_H

#include "demography.h"
#include "list.h"

typedef struct AppContext AppContext;

typedef struct {
    double x;
    double y;
} GraphPoint;

typedef struct {
    int start;
    int end;
} YearInfo;

typedef struct {
    double min;
    double max;
    double mediana;
} Metrix;

typedef struct {
    const char* region;
    Column column;
    YearInfo years;
} FilterCriteria;

double getValueByColumn(DemographicRecord* record, Column column);
Metrix calculateMetrix(AppContext* context, const char* region, Column columnIndex, YearInfo yearInfo);

#endif // METRIX_H
