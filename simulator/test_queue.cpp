#include "mocks/freertos/FreeRTOS.h"
#include "mocks/freertos/queue.h"
#include <iostream>
#include <cassert>
#include <pthread.h>

void test_basic_queue() {
    QueueHandle_t queue = xQueueCreate(5, sizeof(int));
    assert(queue != nullptr);

    int value = 42;
    BaseType_t result = xQueueSend(queue, &value, 0);
    assert(result == pdPASS);

    int received = 0;
    result = xQueueReceive(queue, &received, 0);
    assert(result == pdPASS);
    assert(received == 42);

    vQueueDelete(queue);
    std::cout << "✓ test_basic_queue" << std::endl;
}

void test_queue_full() {
    QueueHandle_t queue = xQueueCreate(2, sizeof(int));

    int value = 1;
    assert(xQueueSend(queue, &value, 0) == pdPASS);
    value = 2;
    assert(xQueueSend(queue, &value, 0) == pdPASS);
    value = 3;
    assert(xQueueSend(queue, &value, 0) == pdFAIL);  // Queue full

    vQueueDelete(queue);
    std::cout << "✓ test_queue_full" << std::endl;
}

void test_queue_empty() {
    QueueHandle_t queue = xQueueCreate(2, sizeof(int));

    int value;
    assert(xQueueReceive(queue, &value, 0) == pdFAIL);  // Queue empty

    vQueueDelete(queue);
    std::cout << "✓ test_queue_empty" << std::endl;
}

void* producer_thread(void* arg) {
    QueueHandle_t queue = static_cast<QueueHandle_t>(arg);
    for (int i = 0; i < 10; i++) {
        xQueueSend(queue, &i, portMAX_DELAY);
    }
    return nullptr;
}

void* consumer_thread(void* arg) {
    QueueHandle_t queue = static_cast<QueueHandle_t>(arg);
    for (int i = 0; i < 10; i++) {
        int value;
        xQueueReceive(queue, &value, portMAX_DELAY);
        assert(value == i);
    }
    return nullptr;
}

void test_threaded_queue() {
    QueueHandle_t queue = xQueueCreate(3, sizeof(int));

    pthread_t producer, consumer;
    pthread_create(&producer, nullptr, producer_thread, queue);
    pthread_create(&consumer, nullptr, consumer_thread, queue);

    pthread_join(producer, nullptr);
    pthread_join(consumer, nullptr);

    vQueueDelete(queue);
    std::cout << "✓ test_threaded_queue" << std::endl;
}

int main() {
    test_basic_queue();
    test_queue_full();
    test_queue_empty();
    test_threaded_queue();
    std::cout << "All queue tests passed!" << std::endl;
    return 0;
}
