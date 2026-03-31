#include "metrix.h"
#include "iterator.h"
#include <string.h>
#include "appcontext.h"
#include "demography.h"

#define INIT_CAPACITY 10

double getValueByColumn(DemographicRecord* record, Column column) {
  double value = 0;
  switch (column) {
    case COL_YEAR:
      value = record->year;
      break;
    case COL_NPG:
      value = record->natural_population_growth;
      break;
    case COL_BIRTH_RATE:
      value = record->birth_rate;
      break;
    case COL_DEATH_RATE:
      value = record->death_rate;
      break;
    case COL_GDW:
      value = record->general_demographic_weight;
      break;
    case COL_URBANIZATION:
      value = record->urbanization;
      break;
    default:
      break; // nice
  }
  return value;
}

int checkColumn(AppContext* context, Column column) {
  int isCorrect = 1;
  if (column < COL_YEAR || column > COL_URBANIZATION || column == COL_REGION) {
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
  return p1->year - p2->year;
}

int fillSortedData(LinkedList* sourceList, LinkedList* resList, LinkedList* pointsList, const char* reg, Column col) {
  int isCorrect = 1;
  Iterator it = begin(sourceList);
  while(hasNext(&it) && isCorrect) {
    DemographicRecord* record = (DemographicRecord*)get(&it);
    if (strcmp(reg, record->region) == 0) { //
        double val = getValueByColumn(record, col);
        GraphPoint point = {0}; // // with debugger
        point.year = record->year;
        point.value = val;
        if (!insertSort(resList, &val, compareDoubles) ||
            !insertSort(pointsList, &point, compareGraphPoints)) {
          isCorrect = 0;
        }
    }
    next(&it);
  }
  return isCorrect;
}

void findMetrix(LinkedList* list, Metrix* metrix) {
  metrix->min = *(double*)list->head->data;
  metrix->max = *(double*)list->tail->data;

  int mid = list->size / 2;
  Iterator it = begin(list);
  for (int i = 0; i < mid; i++)
    next(&it);
  if (list->size % 2 != 0)
    metrix->mediana = *(double*)get(&it);
  else {
    double valRight = *(double*)get(&it);
    double valLeft = *(double*)it.current->prev->data; //
    metrix->mediana = (valLeft + valRight) / 2.0;
  }
}

Metrix calculateMetrix(AppContext* context, const char* region, Column column) {
  Metrix metrix = {0}; //
  LinkedList* tempList = NULL;
  if (context != NULL && context->list != NULL && region != NULL && checkColumn(context, column)) {

    if (context->metrix.graphPoints != NULL) {
      disposeList(context->metrix.graphPoints);
      context->metrix.graphPoints = NULL;
    }

    tempList = initLinkedList(sizeof(double));
    metrix.graphPoints = initLinkedList(sizeof(GraphPoint));
    if (tempList != NULL && metrix.graphPoints != NULL &&
        fillSortedData(context->list, tempList, metrix.graphPoints, region, column)) {

      if (tempList->size > 0) {
        findMetrix(tempList, &metrix);
        context->programmStatus = STATUS_OK;
      } else
          context->programmStatus = ERR_INVALID_REGION;

    } else
        context->programmStatus = ERR_MALLOC_FAILED;
    disposeList(tempList);
    if (context->programmStatus != STATUS_OK && metrix.graphPoints != NULL) {
      disposeList(metrix.graphPoints);
      metrix.graphPoints = NULL;
    }
  }
  return metrix;
}
