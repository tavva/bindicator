#ifndef FREERTOS_TASK_H
#define FREERTOS_TASK_H

#include "FreeRTOS.h"

// Task-related types and defines
typedef void (*TaskFunction_t)(void*);
typedef void* TaskHandle_t;

// Common task functions that might be needed
void vTaskDelay(TickType_t xTicksToDelay);
BaseType_t xTaskCreate(TaskFunction_t pvTaskCode,
                      const char* pcName,
                      uint32_t usStackDepth,
                      void* pvParameters,
                      UBaseType_t uxPriority,
                      TaskHandle_t* pxCreatedTask);

#endif
