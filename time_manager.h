#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>

// Initialize and sync time with NTP
bool setupTime();

// Check if time is valid/set
bool isTimeValid();

#endif
