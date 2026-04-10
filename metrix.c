#include "metrix.h"
#include "iterator.h"
#include <string.h>
#include "appcontext.h"
#include "demography.h"

#define INIT_CAPACITY 10

double getValueByColumn(DemographicRecord* record, Column column) {
  double value = 0;
  switch (column) {
    case YEAR:
      value = record->year;
      break;
    case NPG:
      value = record->natural_population_growth;
      break;
    case BIRTH_RATE:
      value = record->birth_rate;
      break;
    case DEATH_RATE:
      value = record->death_rate;
      break;
    case GDW:
      value = record->general_demographic_weight;
      break;
    case URBANIZATION:
      value = record->urbanization;
      break;
    default:
      break; // nice
  }
  return value;
}

int checkColumn(AppContext* context, Column column) {
  int isCorrect = 1;
  if (column < YEAR || column > URBANIZATION || column == REGION) {
    context->programmStatus = ERR_INVALID_COLUMN;
    isCorrect = 0;
  }
  return isCorrect;
}

int compareDoubles(const void* a, const void* b) {
  double d1 = *(const double*)a;
  double d2 = *(const double*)b;
  return (d1 > d2) - (d1 < d2);
}

int compareGraphPoints(const void* a, const void* b) {
  const GraphPoint* p1 = (const GraphPoint*)a;
  const GraphPoint* p2 = (const GraphPoint*)b;
  return p1->x - p2->x;
}

int isRecordMatches(const DemographicRecord* record, const FilterCriteria* filter) {
  int matches = (strcmp(filter->region, record->region) == 0 &&
  record->year >= filter->years.start &&
  record->year <= filter->years.end);
  return matches;
}

int processAndInsertRecord(const DemographicRecord* record, const FilterCriteria* filter, LinkedList* resList, LinkedList* pointsList) {
  int isCorrect = 0;
  double val = getValueByColumn((DemographicRecord*)record, filter->column);
  GraphPoint point = {(double)record->year, val};
  if (insertSort(resList, &val, compareDoubles)) {
    if (insertSort(pointsList, &point, compareGraphPoints))
      isCorrect = 1;
  }
  return isCorrect;
}

int fillSortedData(LinkedList* sourceList, LinkedList* resList, LinkedList* pointsList, const FilterCriteria* filter) {
  int isCorrect = 1;
  Iterator it = begin(sourceList);
  while (isCorrect && hasNext(&it)) {
    DemographicRecord* record = (DemographicRecord*)get(&it);
    if (isRecordMatches(record, filter))
      isCorrect = processAndInsertRecord(record, filter, resList, pointsList);
    if (isCorrect)
      next(&it);
  }
  return isCorrect;
}

Metrix buildMetrix(LinkedList* list) {
  Metrix metrix;
  metrix.min = *(double*)getByIndex(list, 0);
  metrix.max = *(double*)getByIndex(list, list->size - 1);

  int mid = list->size / 2;
  if (list->size % 2 != 0)
    metrix.mediana = *(double*)getByIndex(list, mid);
  else {
    double valLeft = *(double*)getByIndex(list, mid - 1);
    double valRight = *(double*)getByIndex(list, mid);
    metrix.mediana = (valLeft + valRight) / 2.0;
  }
  return metrix;
}

Metrix calculateMetrix(AppContext* context, const char* region, Column column, YearInfo years) {
  Metrix metrix = {0};
  LinkedList* tempList = NULL;
  FilterCriteria filter = {.region = region, .column = column, .years = years};

  if (context != NULL && context->list != NULL && region != NULL && checkColumn(context, column)) {

    if (isEmpty(context->graphPoints))
      context->graphPoints = initLinkedList(sizeof(GraphPoint));
    else
      clearList(context->graphPoints);

    tempList = initLinkedList(sizeof(double));

    if (tempList != NULL && context->graphPoints != NULL &&
                        fillSortedData(context->list, tempList, context->graphPoints, &filter)) {

      if (tempList->size > 0) {
        metrix = buildMetrix(tempList);
        context->programmStatus = OK;
      } else
          context->programmStatus = ERR_INVALID_REGION;
    } else
        context->programmStatus = ERR_MALLOC_FAILED;
    disposeList(tempList);
    if (context->programmStatus != OK && context->graphPoints != NULL) {
      disposeList(context->graphPoints);
      context->graphPoints = NULL;
    }
  }
  return metrix;
}
