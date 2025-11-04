#pragma once

#include "FreeRTOS.h"
#include <pthread.h>
#include <deque>

struct QueueDefinition {
    std::deque<uint8_t*> items;
    size_t itemSize;
    size_t maxItems;
    pthread_mutex_t mutex;
    pthread_cond_t notEmpty;
    pthread_cond_t notFull;
};

QueueHandle_t xQueueCreate(size_t queueLength, size_t itemSize);
BaseType_t xQueueSend(QueueHandle_t queue, const void* item, TickType_t ticksToWait);
BaseType_t xQueueReceive(QueueHandle_t queue, void* buffer, TickType_t ticksToWait);
void vQueueDelete(QueueHandle_t queue);
