#ifndef BINDICATOR_STATE_H
#define BINDICATOR_STATE_H

enum class BindicatorState {
    NO_COLLECTION,      // No bin collection today
    RECYCLING_DUE,      // Recycling bin needs to be taken out
    RUBBISH_DUE,        // Rubbish bin needs to be taken out
    COMPLETED,          // Bin has been taken out
    LOADING,            // Initial state or checking calendar
    SETUP,              // Device is in setup mode
    ERROR_API,          // API error state
    ERROR_WIFI          // WiFi error state
};

#endif
