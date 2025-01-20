#ifndef FREERTOS_MOCK_H
#define FREERTOS_MOCK_H

#include <queue>

typedef void* QueueHandle_t;
typedef int BaseType_t;

#define pdTRUE 1
#define pdFALSE 0
#define portMAX_DELAY 0xFFFFFFFF

struct QueueData {
    std::queue<void*> items;
    size_t itemSize;
};

QueueHandle_t xQueueCreate(int length, size_t itemSize);
void vQueueDelete(QueueHandle_t xQueue);
BaseType_t xQueueSend(QueueHandle_t xQueue, const void* pvItemToQueue, int xTicksToWait);
BaseType_t xQueueReceive(QueueHandle_t xQueue, void* pvBuffer, int xTicksToWait);

#endif
