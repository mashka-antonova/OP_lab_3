#ifndef METRIX_H
#define METRIX_H

#include "demography.h"
#include "list.h"

typedef struct AppContext AppContext;

typedef struct {
    int year;
    double value;
} GraphPoint; // //

typedef struct {
    double min;
    double max;
    double mediana;
    LinkedList* graphPoints; //
} Metrix;

double getValueByColumn(DemographicRecord* record, Column column);
Metrix calculateMetrix(AppContext* context, const char* region, Column columnIndex);

#endif // METRIX_H
