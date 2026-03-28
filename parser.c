#include "parser.h"
#include <stdio.h>
#include <string.h>

int validateHeader(const char* headerLine) {
  int isCorrect = 0;

  if (headerLine != NULL) {
    if (strstr(headerLine, "year") != NULL &&
        strstr(headerLine, "region") != NULL &&
        strstr(headerLine, "npg") != NULL &&
        strstr(headerLine, "birth_rate") != NULL &&
        strstr(headerLine, "death_rate") != NULL &&
        strstr(headerLine, "gdw") != NULL &&
        strstr(headerLine, "urbanization") != NULL)
          isCorrect = 1;
  }

  return isCorrect;
}

int parseDemographyLine(char* line, DemographicRecord* record) {
  int isCorrect = 0;
  int scannedFields = 0;

  if (line != NULL && record != NULL) {
    scannedFields = sscanf(line, "%d,%127[^,],%lf,%lf,%lf,%lf,%lf",
      &record->year,
      record->region,
      &record->natural_population_growth,
      &record->birth_rate,
      &record->death_rate,
      &record->general_demographic_weight,
      &record->urbanization);

      if (scannedFields == FIELDS_COUNT)
        isCorrect = 1;
    }

    return isCorrect;
}
