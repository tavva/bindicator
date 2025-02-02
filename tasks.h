#ifndef TASKS_H
#define TASKS_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

enum Command {
    CMD_NONE = -1,        // Special value for no command
    CMD_SHOW_NEITHER = 0,
    CMD_SHOW_RECYCLING = 1,
    CMD_SHOW_RUBBISH = 2,
    CMD_SHOW_COMPLETED = 3,
    CMD_SHOW_LOADING = 4,
    CMD_SHOW_ERROR_API = 5,
    CMD_SHOW_ERROR_WIFI = 6,
    CMD_SHOW_SETUP_MODE = 7,
    CMD_SHOW_ERROR_OTHER = 8
};

extern bool isBin;
extern uint8_t Matrix_Data[8][8];
extern uint8_t RGB_Data1[64][3];

void animationTask(void* parameter);
void wifiTask(void* parameter);
void calendarTask(void* parameter);

extern QueueHandle_t commandQueue;

#endif
