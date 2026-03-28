#ifndef DEMOGRAPHY_H
#define DEMOGRAPHY_H

#define MAX_REGION_NAME 128

typedef enum {
    COL_YEAR,
    COL_REGION,
    COL_NPG,
    COL_BIRTH_RATE,
    COL_DEATH_RATE,
    COL_GDW,
    COL_URBANIZATION,
} Column;

typedef struct {
    int year;
    char region[MAX_REGION_NAME];
    double natural_population_growth;
    double birth_rate;
    double death_rate;
    double general_demographic_weight;
    double urbanization;

} DemographicRecord;

#endif // DEMOGRAPHY_H
