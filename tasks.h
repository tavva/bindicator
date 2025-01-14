#ifndef TASKS_H
#define TASKS_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

enum Command {
    CMD_NONE = 0,
    CMD_WIFI_CONNECTED,
    CMD_WIFI_DISCONNECTED,
    CMD_START_PULSE,
    CMD_STOP_PULSE,
    CMD_SHOW_RECYCLING,
    CMD_SHOW_RUBBISH,
    CMD_SHOW_NEITHER
};

extern uint8_t Matrix_Data[8][8];
extern uint8_t RGB_Data1[64][3];

void animationTask(void* parameter);
void wifiTask(void* parameter);
void calendarTask(void* parameter);

extern QueueHandle_t commandQueue;

#endif
