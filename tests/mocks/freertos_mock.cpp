#include "freertos_mock.h"
#include <cstring>
#include <map>

static std::map<QueueHandle_t, QueueData> queues;

QueueHandle_t xQueueCreate(int length, size_t itemSize) {
    QueueData data;
    data.itemSize = itemSize;
    QueueHandle_t handle = new int(1);  // Simple unique handle
    queues[handle] = data;
    return handle;
}

void vQueueDelete(QueueHandle_t xQueue) {
    queues.erase(xQueue);
    delete static_cast<int*>(xQueue);
}

BaseType_t xQueueSend(QueueHandle_t xQueue, const void* pvItemToQueue, int xTicksToWait) {
    auto& queue = queues[xQueue];
    void* item = malloc(queue.itemSize);
    memcpy(item, pvItemToQueue, queue.itemSize);
    queue.items.push(item);
    return pdTRUE;
}

BaseType_t xQueueReceive(QueueHandle_t xQueue, void* pvBuffer, int xTicksToWait) {
    auto& queue = queues[xQueue];
    if (queue.items.empty()) {
        return pdFALSE;
    }

    void* item = queue.items.front();
    memcpy(pvBuffer, item, queue.itemSize);
    free(item);
    queue.items.pop();
    return pdTRUE;
}
