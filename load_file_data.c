#include "load_file_data.h"
#include "parser.h"
#include <stdio.h>
#include "logic.h"

void processLines(AppContext* context, FILE* file) {
  char buffer[MAX_BUFFER_SIZE];
  while (fgets(buffer, sizeof(buffer), file)) {
    DemographicRecord record;
    context->stats.totalRows++;
    if (parseDemographyLine(buffer, &record))
      insertSort(context->list, &record, compareRecords);
    else
      context->stats.errorRows++;
    }
}

int loadDemographyData(AppContext* context, const char* fileName) {
  int isCorrect = 0;
  char buffer[MAX_BUFFER_SIZE];

  if (context != NULL && fileName != NULL) {
    FILE* file = fopen(fileName, "r");

    if (file != NULL) {
      if (fgets(buffer, sizeof(buffer), file) && validateHeader(buffer)) {
          processLines(context, file);
          isCorrect = 1;
      }
      fclose(file);
    }
  }

  return isCorrect;
}
