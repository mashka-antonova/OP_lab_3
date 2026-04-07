#ifndef DEMOGRAPHY_H
#define DEMOGRAPHY_H

#define MAX_REGION_NAME 128

typedef enum {
    YEAR,
    REGION,
    NPG,
    BIRTH_RATE,
    DEATH_RATE,
    GDW,
    URBANIZATION,
    COUNT
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
