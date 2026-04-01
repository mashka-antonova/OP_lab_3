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
    int startYear;
    int endYear;
} YearInfo;

typedef struct {
    double min;
    double max;
    double mediana;
} Metrix;

double getValueByColumn(DemographicRecord* record, Column column);
Metrix calculateMetrix(AppContext* context, const char* region, Column columnIndex, YearInfo yearInfo);

#endif // METRIX_H
