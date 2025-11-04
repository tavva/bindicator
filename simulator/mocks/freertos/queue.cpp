// ABOUTME: FreeRTOS queue implementation using pthreads
// ABOUTME: Provides thread-safe message passing between simulated tasks

#include "queue.h"
#include <cstring>
#include <cstdlib>
#include <sys/time.h>

QueueHandle_t xQueueCreate(size_t queueLength, size_t itemSize) {
    QueueDefinition* queue = new QueueDefinition();
    queue->itemSize = itemSize;
    queue->maxItems = queueLength;
    pthread_mutex_init(&queue->mutex, nullptr);
    pthread_cond_init(&queue->notEmpty, nullptr);
    pthread_cond_init(&queue->notFull, nullptr);
    return queue;
}

BaseType_t xQueueSend(QueueHandle_t handle, const void* item, TickType_t ticksToWait) {
    QueueDefinition* queue = static_cast<QueueDefinition*>(handle);

    pthread_mutex_lock(&queue->mutex);

    // If queue is full
    if (queue->items.size() >= queue->maxItems) {
        if (ticksToWait == 0) {
            pthread_mutex_unlock(&queue->mutex);
            return pdFAIL;
        }

        if (ticksToWait == portMAX_DELAY) {
            // Block indefinitely
            while (queue->items.size() >= queue->maxItems) {
                pthread_cond_wait(&queue->notFull, &queue->mutex);
            }
        } else {
            // Block with timeout
            struct timespec ts;
            struct timeval tv;
            gettimeofday(&tv, nullptr);
            ts.tv_sec = tv.tv_sec + (ticksToWait / 1000);
            ts.tv_nsec = (tv.tv_usec * 1000) + ((ticksToWait % 1000) * 1000000);

            if (pthread_cond_timedwait(&queue->notFull, &queue->mutex, &ts) != 0) {
                pthread_mutex_unlock(&queue->mutex);
                return pdFAIL;
            }
        }
    }

    // Copy item into queue
    uint8_t* itemCopy = new uint8_t[queue->itemSize];
    memcpy(itemCopy, item, queue->itemSize);
    queue->items.push_back(itemCopy);

    pthread_cond_signal(&queue->notEmpty);
    pthread_mutex_unlock(&queue->mutex);

    return pdPASS;
}

BaseType_t xQueueReceive(QueueHandle_t handle, void* buffer, TickType_t ticksToWait) {
    QueueDefinition* queue = static_cast<QueueDefinition*>(handle);

    pthread_mutex_lock(&queue->mutex);

    // If queue is empty
    if (queue->items.empty()) {
        if (ticksToWait == 0) {
            pthread_mutex_unlock(&queue->mutex);
            return pdFAIL;
        }

        if (ticksToWait == portMAX_DELAY) {
            // Block indefinitely
            while (queue->items.empty()) {
                pthread_cond_wait(&queue->notEmpty, &queue->mutex);
            }
        } else {
            // Block with timeout
            struct timespec ts;
            struct timeval tv;
            gettimeofday(&tv, nullptr);
            ts.tv_sec = tv.tv_sec + (ticksToWait / 1000);
            ts.tv_nsec = (tv.tv_usec * 1000) + ((ticksToWait % 1000) * 1000000);

            if (pthread_cond_timedwait(&queue->notEmpty, &queue->mutex, &ts) != 0) {
                pthread_mutex_unlock(&queue->mutex);
                return pdFAIL;
            }
        }
    }

    // Copy item from queue
    uint8_t* item = queue->items.front();
    queue->items.pop_front();
    memcpy(buffer, item, queue->itemSize);
    delete[] item;

    pthread_cond_signal(&queue->notFull);
    pthread_mutex_unlock(&queue->mutex);

    return pdPASS;
}

void vQueueDelete(QueueHandle_t handle) {
    QueueDefinition* queue = static_cast<QueueDefinition*>(handle);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->notEmpty);
    pthread_cond_destroy(&queue->notFull);

    // Clean up remaining items
    for (auto item : queue->items) {
        delete[] item;
    }

    delete queue;
}
