#pragma once

#include "FreeRTOS.h"
#include <pthread.h>

typedef void (*TaskFunction_t)(void*);

BaseType_t xTaskCreate(
    TaskFunction_t taskFunction,
    const char* name,
    uint32_t stackDepth,
    void* parameters,
    uint32_t priority,
    TaskHandle_t* handle
);

BaseType_t xTaskCreatePinnedToCore(
    TaskFunction_t taskFunction,
    const char* name,
    uint32_t stackDepth,
    void* parameters,
    uint32_t priority,
    TaskHandle_t* handle,
    uint32_t coreId
);

void vTaskDelay(TickType_t ticks);
void vTaskDelayUntil(TickType_t* previousWakeTime, TickType_t timeIncrement);
TickType_t xTaskGetTickCount();

// Semaphore functions (minimal)
inline SemaphoreHandle_t xSemaphoreCreateMutex() {
    pthread_mutex_t* mutex = new pthread_mutex_t();
    pthread_mutex_init(mutex, nullptr);
    return mutex;
}

inline BaseType_t xSemaphoreTake(SemaphoreHandle_t semaphore, TickType_t ticksToWait) {
    pthread_mutex_t* mutex = static_cast<pthread_mutex_t*>(semaphore);
    return pthread_mutex_lock(mutex) == 0 ? pdTRUE : pdFALSE;
}

inline BaseType_t xSemaphoreGive(SemaphoreHandle_t semaphore) {
    pthread_mutex_t* mutex = static_cast<pthread_mutex_t*>(semaphore);
    return pthread_mutex_unlock(mutex) == 0 ? pdTRUE : pdFALSE;
}
