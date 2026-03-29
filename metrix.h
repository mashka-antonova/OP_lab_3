#ifndef METRIX_H
#define METRIX_H

#include "demography.h"

typedef struct AppContext AppContext;

typedef struct {
    double min;
    double max;
    double mediana;
} Metrix;

double getValueByColumn(DemographicRecord* record, Column column);
Metrix calculateMetrix(AppContext* context, const char* region, Column columnIndex);

#endif // METRIX_H
