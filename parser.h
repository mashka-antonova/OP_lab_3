#ifndef PARSER_H
#define PARSER_H

#include "demography.h"

int validateHeader(const char* headerLine);
int parseDemographyLine(char* line, DemographicRecord* record);

#endif // PARSER_H
