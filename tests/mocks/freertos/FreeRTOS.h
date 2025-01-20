#ifndef FREERTOS_H
#define FREERTOS_H

#include "../freertos_mock.h"

// Common FreeRTOS types
typedef unsigned int TickType_t;
typedef unsigned int UBaseType_t;
typedef int BaseType_t;

// Common macros
#define pdMS_TO_TICKS(xTimeInMs) ((TickType_t)(xTimeInMs))
#define configMAX_PRIORITIES 25

#endif
