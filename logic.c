#include "logic.h"
#include "load_file_data.h"
#include "demography.h"
#include <string.h>

void initContext(AppContext* context) {
  if (context != NULL) {
    context->list = NULL;
    context->rowsInfo.totalRows = 0;
    context->rowsInfo.invalidRows = 0;
    context->programmStatus = OK;
    context->metrix.min = 0;
    context->metrix.max = 0;
    context->metrix.mediana = 0;
    context->graphPoints = NULL;
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

  context->rowsInfo.totalRows = 0;
  context->rowsInfo.invalidRows = 0;

  if (!loadDemographyData(context, fileName))
    context->programmStatus = ERR_FILE_OPEN;
}

void runCalculateMetricsTask(AppContext* context, const char* region, Column column, YearInfo years) {
  context->programmStatus = OK;
  context->metrix = calculateMetrix(context, region, column, years);
}

void disposeContext(AppContext* context) {
  if (context != NULL) {
    if (context->list != NULL)
      disposeList(context->list);
    if (context->graphPoints != NULL)
      disposeList(context->graphPoints);
    initContext(context);
  }
}
