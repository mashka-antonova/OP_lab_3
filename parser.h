#ifndef PARSER_H
#define PARSER_H

#include "demography.h"

#define FIELDS_COUNT 7

int validateHeader(const char* headerLine);
int parseDemographyLine(char* line, DemographicRecord* record);

#endif // PARSER_H
