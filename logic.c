#include "logic.h"
#include "load_file_data.h"
#include "demography.h"
#include <string.h>

void initContext(AppContext* context) {
  if (context != NULL) {
    context->list = NULL;
    context->stats.totalRows = 0;
    context->stats.errorRows = 0;
    context->programmStatus = STATUS_OK;
    context->metrix.min = 0;
    context->metrix.max = 0;
    context->metrix.mediana = 0;
    context->metrix.graphPoints = NULL;
  }
}

int compareRecords(const void* a, const void* b) {
    const DemographicRecord* recA = (const DemographicRecord*)a;
    const DemographicRecord* recB = (const DemographicRecord*)b;
    int res = strcmp(recA->region, recB->region);
    if (res == 0)
      res = recA->year - recB->year;
    return res;
}

void runLoadDataTask(AppContext* context, const char* fileName) {
  if (context->list != NULL)
    clearList(context->list);
  else
    context->list = initLinkedList(sizeof(DemographicRecord));

  context->stats.totalRows = 0;
  context->stats.errorRows = 0;

  if (!loadDemographyData(context, fileName))
    context->programmStatus = ERR_FILE_OPEN;
}

void runCalculateMetricsTask(AppContext* context, const char* region, Column column) {
  context->programmStatus = STATUS_OK;
  context->metrix = calculateMetrix(context, region, column);
}

void disposeContext(AppContext* context) {
  if (context != NULL) {
    if (context->list != NULL)
      disposeList(context->list);
    if (context->metrix.graphPoints != NULL)
      disposeList(context->metrix.graphPoints);
    initContext(context);
  }
}
