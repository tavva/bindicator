// ABOUTME: FreeRTOS task implementation using pthreads
// ABOUTME: Maps task creation to native threads with parameter passing

#include "task.h"
#include "../Arduino.h"
#include <unistd.h>

struct TaskParams {
    TaskFunction_t function;
    void* parameter;
};

static void* taskWrapper(void* arg) {
    TaskParams* params = static_cast<TaskParams*>(arg);
    params->function(params->parameter);
    delete params;
    return nullptr;
}

BaseType_t xTaskCreate(
    TaskFunction_t taskFunction,
    const char* name,
    uint32_t stackDepth,
    void* parameters,
    uint32_t priority,
    TaskHandle_t* handle
) {
    TaskParams* params = new TaskParams{taskFunction, parameters};
    pthread_t* thread = new pthread_t();

    if (pthread_create(thread, nullptr, taskWrapper, params) != 0) {
        delete params;
        delete thread;
        return pdFAIL;
    }

    if (handle != nullptr) {
        *handle = thread;
    }

    return pdPASS;
}

BaseType_t xTaskCreatePinnedToCore(
    TaskFunction_t taskFunction,
    const char* name,
    uint32_t stackDepth,
    void* parameters,
    uint32_t priority,
    TaskHandle_t* handle,
    uint32_t coreId
) {
    // Core pinning not supported on macOS, just create task
    return xTaskCreate(taskFunction, name, stackDepth, parameters, priority, handle);
}

void vTaskDelay(TickType_t ticks) {
    SimulatedTime::advance(ticks);
    usleep(10000);  // Small real delay to allow other threads to run
}

void vTaskDelayUntil(TickType_t* previousWakeTime, TickType_t timeIncrement) {
    TickType_t currentTime = xTaskGetTickCount();
    TickType_t timeToWake = *previousWakeTime + timeIncrement;

    if (timeToWake > currentTime) {
        vTaskDelay(timeToWake - currentTime);
    }

    *previousWakeTime = timeToWake;
}

TickType_t xTaskGetTickCount() {
    return SimulatedTime::millis();
}
