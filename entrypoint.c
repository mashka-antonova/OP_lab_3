#include "entrypoint.h"
#include "logic.h"

void doOperation(Operation operation, AppContext* context, AppParams* params) {
  switch (operation) {
    case LOAD_DATA:
      runLoadDataTask(context, params->str);
      break;

    case CALCULATE_METRICS:
      runCalculateMetricsTask(context, params->str, params->column);
      break;

    case INITIALIZATION:
      initContext(context);
      break;
  }
}
