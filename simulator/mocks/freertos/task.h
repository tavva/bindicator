#pragma once

#include "FreeRTOS.h"
#include <pthread.h>
#include <unistd.h>

typedef void (*TaskFunction_t)(void*);

inline BaseType_t xTaskCreate(
    TaskFunction_t taskFunction,
    const char* name,
    uint32_t stackDepth,
    void* parameters,
    uint32_t priority,
    TaskHandle_t* handle
) {
    // Will implement in next task
    return pdFAIL;
}

inline BaseType_t xTaskCreatePinnedToCore(
    TaskFunction_t taskFunction,
    const char* name,
    uint32_t stackDepth,
    void* parameters,
    uint32_t priority,
    TaskHandle_t* handle,
    uint32_t coreId
) {
    // Will implement in next task
    return pdFAIL;
}

inline void vTaskDelay(TickType_t ticks) {
    usleep(ticks * 1000);
}

inline void vTaskDelayUntil(TickType_t* previousWakeTime, TickType_t timeIncrement) {
    vTaskDelay(timeIncrement);
}

inline TickType_t xTaskGetTickCount() {
    return 0;  // Will implement with simulated time
}
