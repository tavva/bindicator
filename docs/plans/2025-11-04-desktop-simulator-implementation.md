# Desktop Simulator Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build a desktop simulator that runs actual Bindicator firmware on macOS/Linux with comprehensive mocks for ESP32/Arduino APIs, enabling rapid development, debugging, and automated testing.

**Architecture:** The simulator compiles firmware source files (bindicator.cpp, oauth_handler.cpp, etc.) against a mock layer that provides ESP32/Arduino APIs using native libraries (pthreads for FreeRTOS, libcurl for HTTP, JSON files for Preferences). An interactive runtime provides terminal visualization and time control commands.

**Tech Stack:** C++14, pthreads, libcurl, JSON for mock responses, ANSI terminal graphics

---

## Phase 1: Core Infrastructure

### Task 1: Project Structure and Basic Build System

**Files:**
- Create: `simulator/main.cpp`
- Create: `simulator/Makefile`
- Modify: `Makefile` (root)
- Create: `.gitignore` entries

**Step 1: Create simulator directory structure**

```bash
mkdir -p simulator/mocks/freertos
mkdir -p simulator/mock_responses/oauth
mkdir -p simulator/mock_responses/calendar
```

**Step 2: Create basic main.cpp**

Create `simulator/main.cpp`:
```cpp
// ABOUTME: Entry point for Bindicator desktop simulator
// ABOUTME: Provides interactive runtime for testing firmware without hardware

#include <iostream>
#include <string>

int main(int argc, char** argv) {
    std::cout << "Bindicator Simulator v0.1" << std::endl;
    std::cout << "Type 'help' for commands" << std::endl;

    std::string command;
    while (std::getline(std::cin, command)) {
        if (command == "quit" || command == "exit") {
            break;
        } else if (command == "help") {
            std::cout << "Available commands:" << std::endl;
            std::cout << "  help  - Show this message" << std::endl;
            std::cout << "  quit  - Exit simulator" << std::endl;
        } else {
            std::cout << "Unknown command: " << command << std::endl;
        }
    }

    return 0;
}
```

**Step 3: Create simulator Makefile**

Create `simulator/Makefile`:
```makefile
CXX = g++
CXXFLAGS = -std=c++14 -Wall -Werror -DSIMULATOR
CXXFLAGS += -Imocks -Imocks/freertos -I..
LDFLAGS = -pthread -lcurl

TARGET = simulator
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)
```

**Step 4: Add root Makefile targets**

Add to root `Makefile` after existing targets:
```makefile
# Simulator targets
simulator:
	cd simulator && $(MAKE)

sim-run:
	cd simulator && $(MAKE) run

sim-clean:
	cd simulator && $(MAKE) clean

.PHONY: simulator sim-run sim-clean
```

**Step 5: Update .gitignore**

Add to `.gitignore`:
```
# Simulator
simulator/simulator
simulator/*.o
simulator/mocks/*.o
simulator/mocks/freertos/*.o
simulator-state.json
.env
```

**Step 6: Build and test**

```bash
make simulator
./simulator/simulator
# Type "help", then "quit"
```

Expected: Compiles cleanly, runs, accepts commands

**Step 7: Commit**

```bash
git add simulator/ Makefile .gitignore
git commit -m "feat: add simulator project structure and basic build system"
```

---

### Task 2: Simulated Time System

**Files:**
- Create: `simulator/simulated_time.h`
- Create: `simulator/simulated_time.cpp`
- Create: `simulator/test_simulated_time.cpp`

**Step 1: Write test for time simulation**

Create `simulator/test_simulated_time.cpp`:
```cpp
#include <iostream>
#include <cassert>
#include "simulated_time.h"

void test_millis_basic() {
    SimulatedTime::reset();
    unsigned long t1 = SimulatedTime::millis();
    SimulatedTime::advance(1000);
    unsigned long t2 = SimulatedTime::millis();
    assert(t2 - t1 == 1000);
    std::cout << "✓ test_millis_basic" << std::endl;
}

void test_time_multiplier() {
    SimulatedTime::reset();
    SimulatedTime::setMultiplier(10.0);
    unsigned long t1 = SimulatedTime::millis();
    SimulatedTime::advance(100);  // Real time
    unsigned long t2 = SimulatedTime::millis();
    assert(t2 - t1 == 1000);  // Simulated time (10x)
    std::cout << "✓ test_time_multiplier" << std::endl;
}

void test_millis_overflow() {
    SimulatedTime::reset();
    SimulatedTime::setTime(0xFFFFFF00);
    unsigned long t1 = SimulatedTime::millis();
    SimulatedTime::advance(0x200);
    unsigned long t2 = SimulatedTime::millis();
    // Should wrap around
    assert(t2 < t1);
    assert(t2 == 0x100);
    std::cout << "✓ test_millis_overflow" << std::endl;
}

int main() {
    test_millis_basic();
    test_time_multiplier();
    test_millis_overflow();
    std::cout << "All time tests passed!" << std::endl;
    return 0;
}
```

**Step 2: Run test to verify it fails**

```bash
cd simulator
g++ -std=c++14 -Wall test_simulated_time.cpp -o test_time
```

Expected: Compilation fails with "simulated_time.h: No such file"

**Step 3: Implement simulated time header**

Create `simulator/simulated_time.h`:
```cpp
#pragma once

class SimulatedTime {
public:
    static unsigned long millis();
    static void advance(unsigned long ms);
    static void setTime(unsigned long ms);
    static void setMultiplier(float multiplier);
    static void reset();

private:
    static unsigned long baseTime;
    static float timeMultiplier;
};
```

**Step 4: Implement simulated time**

Create `simulator/simulated_time.cpp`:
```cpp
// ABOUTME: Simulated time system for testing time-dependent firmware behaviour
// ABOUTME: Supports time acceleration, manual advancement, and overflow testing

#include "simulated_time.h"

unsigned long SimulatedTime::baseTime = 0;
float SimulatedTime::timeMultiplier = 1.0f;

unsigned long SimulatedTime::millis() {
    return baseTime;
}

void SimulatedTime::advance(unsigned long ms) {
    baseTime += static_cast<unsigned long>(ms * timeMultiplier);
}

void SimulatedTime::setTime(unsigned long ms) {
    baseTime = ms;
}

void SimulatedTime::setMultiplier(float multiplier) {
    timeMultiplier = multiplier;
}

void SimulatedTime::reset() {
    baseTime = 0;
    timeMultiplier = 1.0f;
}
```

**Step 5: Build and run tests**

```bash
cd simulator
g++ -std=c++14 -Wall test_simulated_time.cpp simulated_time.cpp -o test_time
./test_time
```

Expected: All tests pass

**Step 6: Clean up test binary**

```bash
rm test_time
```

**Step 7: Commit**

```bash
git add simulator/simulated_time.h simulator/simulated_time.cpp simulator/test_simulated_time.cpp
git commit -m "feat: add simulated time system with acceleration and overflow testing"
```

---

### Task 3: Terminal Capability Detection

**Files:**
- Create: `simulator/terminal_display.h`
- Create: `simulator/terminal_display.cpp`

**Step 1: Create terminal display header**

Create `simulator/terminal_display.h`:
```cpp
#pragma once

#include <string>

struct TerminalCapabilities {
    bool hasUTF8;
    bool hasColor;
    bool has256Color;
    int width;
    int height;
};

class TerminalDisplay {
public:
    TerminalDisplay();

    TerminalCapabilities getCapabilities() const { return capabilities; }
    void clear();
    void moveCursor(int x, int y);
    void setColor(int r, int g, int b);
    void resetColor();
    void printBlock();

private:
    TerminalCapabilities detectCapabilities();
    TerminalCapabilities capabilities;
};
```

**Step 2: Implement terminal detection**

Create `simulator/terminal_display.cpp`:
```cpp
// ABOUTME: Terminal display handler with capability detection and graceful degradation
// ABOUTME: Provides ANSI color rendering for LED matrix visualization

#include "terminal_display.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/ioctl.h>
#include <unistd.h>

TerminalDisplay::TerminalDisplay() {
    capabilities = detectCapabilities();

    if (!capabilities.hasUTF8) {
        std::cerr << "Warning: UTF-8 not detected, using ASCII fallback" << std::endl;
    }
    if (!capabilities.hasColor) {
        std::cerr << "Warning: Color support not detected" << std::endl;
    }
}

TerminalCapabilities TerminalDisplay::detectCapabilities() {
    TerminalCapabilities caps;

    // Check UTF-8 support via locale
    const char* lang = getenv("LANG");
    caps.hasUTF8 = (lang != nullptr && strstr(lang, "UTF-8") != nullptr);

    // Check color support via TERM
    const char* term = getenv("TERM");
    caps.hasColor = (term != nullptr &&
                     (strstr(term, "color") != nullptr ||
                      strstr(term, "xterm") != nullptr ||
                      strstr(term, "screen") != nullptr));

    // Check 256 color support
    const char* colorterm = getenv("COLORTERM");
    caps.has256Color = (colorterm != nullptr ||
                        (term != nullptr && strstr(term, "256") != nullptr));

    // Get terminal size
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        caps.width = w.ws_col;
        caps.height = w.ws_row;
    } else {
        caps.width = 80;
        caps.height = 24;
    }

    return caps;
}

void TerminalDisplay::clear() {
    if (capabilities.hasColor) {
        std::cout << "\033[2J\033[H";  // Clear screen and move to home
    } else {
        std::cout << "\n\n\n";
    }
}

void TerminalDisplay::moveCursor(int x, int y) {
    if (capabilities.hasColor) {
        std::cout << "\033[" << y << ";" << x << "H";
    }
}

void TerminalDisplay::setColor(int r, int g, int b) {
    if (capabilities.has256Color) {
        // Use 24-bit color
        std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m";
    } else if (capabilities.hasColor) {
        // Use basic 16 colors (approximate)
        int color = 30;  // Black default
        if (r > 128 && g < 128 && b < 128) color = 31;  // Red
        else if (r < 128 && g > 128 && b < 128) color = 32;  // Green
        else if (r < 128 && g < 128 && b > 128) color = 34;  // Blue
        std::cout << "\033[" << color << "m";
    }
}

void TerminalDisplay::resetColor() {
    if (capabilities.hasColor) {
        std::cout << "\033[0m";
    }
}

void TerminalDisplay::printBlock() {
    if (capabilities.hasUTF8) {
        std::cout << "█";
    } else {
        std::cout << "#";
    }
}
```

**Step 3: Add terminal test to main**

Modify `simulator/main.cpp` to add `test` command:
```cpp
#include "terminal_display.h"

// In main(), add to command handling:
} else if (command == "test") {
    TerminalDisplay display;
    auto caps = display.getCapabilities();
    std::cout << "Terminal capabilities:" << std::endl;
    std::cout << "  UTF-8: " << (caps.hasUTF8 ? "yes" : "no") << std::endl;
    std::cout << "  Color: " << (caps.hasColor ? "yes" : "no") << std::endl;
    std::cout << "  256-color: " << (caps.has256Color ? "yes" : "no") << std::endl;
    std::cout << "  Size: " << caps.width << "x" << caps.height << std::endl;

    display.clear();
    display.moveCursor(1, 1);
    display.setColor(255, 0, 0);
    display.printBlock();
    display.setColor(0, 255, 0);
    display.printBlock();
    display.setColor(0, 0, 255);
    display.printBlock();
    display.resetColor();
    std::cout << " Color test" << std::endl;
```

**Step 4: Update Makefile**

Update `simulator/Makefile` SRCS:
```makefile
SRCS = main.cpp simulated_time.cpp terminal_display.cpp
```

**Step 5: Build and test**

```bash
make simulator
./simulator/simulator
# Type "test" to see terminal capabilities
# Type "quit"
```

Expected: Shows terminal capabilities and colored blocks

**Step 6: Commit**

```bash
git add simulator/terminal_display.h simulator/terminal_display.cpp simulator/main.cpp simulator/Makefile
git commit -m "feat: add terminal capability detection with color support"
```

---

## Phase 2: Mock Layer - Arduino Core

### Task 4: Arduino Mock - Basic Types and Serial

**Files:**
- Create: `simulator/mocks/Arduino.h`
- Create: `simulator/mocks/Arduino.cpp`

**Step 1: Create Arduino.h with basic types**

Create `simulator/mocks/Arduino.h`:
```cpp
#pragma once

#include <cstdint>
#include <string>
#include "../simulated_time.h"

// Arduino basic types
typedef bool boolean;
typedef uint8_t byte;
typedef std::string String;

// Arduino constants
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

// Time functions
inline unsigned long millis() {
    return SimulatedTime::millis();
}

inline void delay(unsigned long ms) {
    SimulatedTime::advance(ms);
}

// Pin functions (no-op for simulator)
inline void pinMode(uint8_t pin, uint8_t mode) {}
inline int digitalRead(uint8_t pin) { return LOW; }
inline void digitalWrite(uint8_t pin, uint8_t val) {}

// Serial class
class SerialClass {
public:
    void begin(unsigned long baud) {}
    void println(const char* str);
    void println(const String& str);
    void print(const char* str);
    void print(const String& str);
    void printf(const char* format, ...);

    bool available() { return false; }
    String readStringUntil(char terminator) { return ""; }
};

extern SerialClass Serial;
```

**Step 2: Implement Serial mock**

Create `simulator/mocks/Arduino.cpp`:
```cpp
// ABOUTME: Arduino core API mocks for desktop simulator
// ABOUTME: Provides Serial output, pin operations, and basic Arduino functions

#include "Arduino.h"
#include <iostream>
#include <cstdarg>
#include <cstdio>

SerialClass Serial;

void SerialClass::println(const char* str) {
    std::cout << str << std::endl;
}

void SerialClass::println(const String& str) {
    std::cout << str << std::endl;
}

void SerialClass::print(const char* str) {
    std::cout << str;
}

void SerialClass::print(const String& str) {
    std::cout << str;
}

void SerialClass::printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
```

**Step 3: Create test file**

Create `simulator/test_arduino.cpp`:
```cpp
#include "mocks/Arduino.h"
#include <cassert>

int main() {
    Serial.begin(115200);
    Serial.println("Testing Serial output");
    Serial.print("Current time: ");
    Serial.println(String(millis()));

    delay(1000);
    assert(millis() == 1000);
    Serial.println("✓ Time simulation works");

    return 0;
}
```

**Step 4: Build and test**

```bash
cd simulator
g++ -std=c++14 -Wall -Imocks test_arduino.cpp mocks/Arduino.cpp simulated_time.cpp -o test_arduino
./test_arduino
rm test_arduino
```

Expected: Prints messages and time advances correctly

**Step 5: Commit**

```bash
git add simulator/mocks/Arduino.h simulator/mocks/Arduino.cpp simulator/test_arduino.cpp
git commit -m "feat: add Arduino core API mocks (Serial, time, pins)"
```

---

### Task 5: FreeRTOS Mock - Queue Implementation

**Files:**
- Create: `simulator/mocks/freertos/FreeRTOS.h`
- Create: `simulator/mocks/freertos/queue.h`
- Create: `simulator/mocks/freertos/queue.cpp`
- Create: `simulator/mocks/freertos/task.h`

**Step 1: Create FreeRTOS basic types**

Create `simulator/mocks/freertos/FreeRTOS.h`:
```cpp
#pragma once

#include <cstdint>

typedef int32_t BaseType_t;
typedef uint32_t TickType_t;
typedef void* TaskHandle_t;
typedef void* QueueHandle_t;
typedef void* SemaphoreHandle_t;

#define pdTRUE 1
#define pdFALSE 0
#define pdPASS 1
#define pdFAIL 0
#define portMAX_DELAY 0xFFFFFFFF

inline TickType_t pdMS_TO_TICKS(uint32_t ms) {
    return ms;
}
```

**Step 2: Create queue header**

Create `simulator/mocks/freertos/queue.h`:
```cpp
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
```

**Step 3: Implement queue**

Create `simulator/mocks/freertos/queue.cpp`:
```cpp
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
```

**Step 4: Create task header (stub for now)**

Create `simulator/mocks/freertos/task.h`:
```cpp
#pragma once

#include "FreeRTOS.h"
#include <pthread.h>
#include <unistd.h>

typedef void (*TaskFunction_t)(void*);

inline BaseType_t xTaskCreate(
    TaskFunction_t taskFunction,
    const char* name,
    uint32_t stackDepth,
    void* parameters,
    uint32_t priority,
    TaskHandle_t* handle
) {
    // Will implement in next task
    return pdFAIL;
}

inline BaseType_t xTaskCreatePinnedToCore(
    TaskFunction_t taskFunction,
    const char* name,
    uint32_t stackDepth,
    void* parameters,
    uint32_t priority,
    TaskHandle_t* handle,
    uint32_t coreId
) {
    // Will implement in next task
    return pdFAIL;
}

inline void vTaskDelay(TickType_t ticks) {
    usleep(ticks * 1000);
}

inline void vTaskDelayUntil(TickType_t* previousWakeTime, TickType_t timeIncrement) {
    vTaskDelay(timeIncrement);
}

inline TickType_t xTaskGetTickCount() {
    return 0;  // Will implement with simulated time
}
```

**Step 5: Create queue test**

Create `simulator/test_queue.cpp`:
```cpp
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
```

**Step 6: Build and run tests**

```bash
cd simulator
g++ -std=c++14 -Wall -Imocks -Imocks/freertos test_queue.cpp mocks/freertos/queue.cpp -pthread -o test_queue
./test_queue
rm test_queue
```

Expected: All tests pass

**Step 7: Commit**

```bash
git add simulator/mocks/freertos/
git commit -m "feat: add FreeRTOS queue implementation using pthreads"
```

---

### Task 6: FreeRTOS Mock - Task Creation

**Files:**
- Modify: `simulator/mocks/freertos/task.h`
- Create: `simulator/mocks/freertos/task.cpp`

**Step 1: Implement task creation**

Modify `simulator/mocks/freertos/task.h` to remove inline implementations and add declarations:
```cpp
#pragma once

#include "FreeRTOS.h"
#include <pthread.h>

typedef void (*TaskFunction_t)(void*);

BaseType_t xTaskCreate(
    TaskFunction_t taskFunction,
    const char* name,
    uint32_t stackDepth,
    void* parameters,
    uint32_t priority,
    TaskHandle_t* handle
);

BaseType_t xTaskCreatePinnedToCore(
    TaskFunction_t taskFunction,
    const char* name,
    uint32_t stackDepth,
    void* parameters,
    uint32_t priority,
    TaskHandle_t* handle,
    uint32_t coreId
);

void vTaskDelay(TickType_t ticks);
void vTaskDelayUntil(TickType_t* previousWakeTime, TickType_t timeIncrement);
TickType_t xTaskGetTickCount();

// Semaphore functions (minimal)
inline SemaphoreHandle_t xSemaphoreCreateMutex() {
    pthread_mutex_t* mutex = new pthread_mutex_t();
    pthread_mutex_init(mutex, nullptr);
    return mutex;
}

inline BaseType_t xSemaphoreTake(SemaphoreHandle_t semaphore, TickType_t ticksToWait) {
    pthread_mutex_t* mutex = static_cast<pthread_mutex_t*>(semaphore);
    return pthread_mutex_lock(mutex) == 0 ? pdTRUE : pdFALSE;
}

inline BaseType_t xSemaphoreGive(SemaphoreHandle_t semaphore) {
    pthread_mutex_t* mutex = static_cast<pthread_mutex_t*>(semaphore);
    return pthread_mutex_unlock(mutex) == 0 ? pdTRUE : pdFALSE;
}
```

**Step 2: Implement task functions**

Create `simulator/mocks/freertos/task.cpp`:
```cpp
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
```

**Step 3: Update Makefile**

Update `simulator/Makefile` SRCS to include new files:
```makefile
SRCS = main.cpp simulated_time.cpp terminal_display.cpp \
       mocks/Arduino.cpp \
       mocks/freertos/queue.cpp mocks/freertos/task.cpp
```

**Step 4: Build simulator**

```bash
make simulator
```

Expected: Compiles successfully

**Step 5: Commit**

```bash
git add simulator/mocks/freertos/task.h simulator/mocks/freertos/task.cpp simulator/Makefile
git commit -m "feat: add FreeRTOS task creation using pthreads"
```

---

## Phase 3: Additional Mocks

### Task 7: Preferences Mock with JSON Backend

**Files:**
- Create: `simulator/mocks/Preferences.h`
- Create: `simulator/mocks/Preferences.cpp`

**Step 1: Create Preferences header**

Create `simulator/mocks/Preferences.h`:
```cpp
#pragma once

#include "Arduino.h"
#include <map>
#include <string>

class Preferences {
public:
    bool begin(const char* name, bool readOnly = false);
    void end();
    void clear();
    bool remove(const char* key);

    String getString(const char* key, const String& defaultValue = "");
    bool putString(const char* key, const String& value);

    int getInt(const char* key, int defaultValue = 0);
    bool putInt(const char* key, int value);

private:
    std::string currentNamespace;
    std::map<std::string, std::map<std::string, std::string>> storage;

    void loadFromFile();
    void saveToFile();
    std::string getFilename();
};
```

**Step 2: Implement Preferences with JSON**

Create `simulator/mocks/Preferences.cpp`:
```cpp
// ABOUTME: ESP32 Preferences mock using JSON file backend
// ABOUTME: Provides persistent storage for WiFi credentials, OAuth tokens, device state

#include "Preferences.h"
#include <fstream>
#include <sstream>
#include <cstdlib>

// Simple JSON parsing/writing (no external lib needed for our simple case)
void Preferences::loadFromFile() {
    std::ifstream file(getFilename());
    if (!file.is_open()) {
        return;  // File doesn't exist yet
    }

    std::string line;
    std::string currentNs;

    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        // Namespace line: [namespace]
        if (line[0] == '[' && line[line.length()-1] == ']') {
            currentNs = line.substr(1, line.length()-2);
            continue;
        }

        // Key-value line: key=value
        size_t pos = line.find('=');
        if (pos != std::string::npos && !currentNs.empty()) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            storage[currentNs][key] = value;
        }
    }
}

void Preferences::saveToFile() {
    std::ofstream file(getFilename());
    if (!file.is_open()) {
        Serial.println("Failed to save preferences");
        return;
    }

    for (const auto& ns : storage) {
        file << "[" << ns.first << "]" << std::endl;
        for (const auto& kv : ns.second) {
            file << kv.first << "=" << kv.second << std::endl;
        }
        file << std::endl;
    }
}

std::string Preferences::getFilename() {
    const char* ephemeral = getenv("SIMULATOR_EPHEMERAL");
    if (ephemeral && std::string(ephemeral) == "1") {
        return "";  // In-memory only
    }
    return "simulator-state.json";
}

bool Preferences::begin(const char* name, bool readOnly) {
    currentNamespace = name;

    if (storage.empty() && !getFilename().empty()) {
        loadFromFile();
    }

    return true;
}

void Preferences::end() {
    // No-op for simulator
}

void Preferences::clear() {
    if (!currentNamespace.empty()) {
        storage[currentNamespace].clear();
        saveToFile();
    }
}

bool Preferences::remove(const char* key) {
    if (currentNamespace.empty()) return false;

    auto it = storage[currentNamespace].find(key);
    if (it != storage[currentNamespace].end()) {
        storage[currentNamespace].erase(it);
        saveToFile();
        return true;
    }
    return false;
}

String Preferences::getString(const char* key, const String& defaultValue) {
    if (currentNamespace.empty()) return defaultValue;

    auto nsIt = storage.find(currentNamespace);
    if (nsIt == storage.end()) return defaultValue;

    auto it = nsIt->second.find(key);
    if (it == nsIt->second.end()) return defaultValue;

    return it->second;
}

bool Preferences::putString(const char* key, const String& value) {
    if (currentNamespace.empty()) return false;

    storage[currentNamespace][key] = value;
    saveToFile();
    return true;
}

int Preferences::getInt(const char* key, int defaultValue) {
    String value = getString(key, "");
    if (value.empty()) return defaultValue;
    return std::stoi(value);
}

bool Preferences::putInt(const char* key, int value) {
    return putString(key, std::to_string(value));
}
```

**Step 3: Update Makefile**

Update SRCS:
```makefile
SRCS = main.cpp simulated_time.cpp terminal_display.cpp \
       mocks/Arduino.cpp mocks/Preferences.cpp \
       mocks/freertos/queue.cpp mocks/freertos/task.cpp
```

**Step 4: Build**

```bash
make simulator
```

Expected: Compiles successfully

**Step 5: Commit**

```bash
git add simulator/mocks/Preferences.h simulator/mocks/Preferences.cpp simulator/Makefile
git commit -m "feat: add Preferences mock with JSON file backend"
```

---

### Task 8: WiFi and HTTPClient Mock Stubs

**Files:**
- Create: `simulator/mocks/WiFi.h`
- Create: `simulator/mocks/WiFi.cpp`
- Create: `simulator/mocks/HTTPClient.h`
- Create: `simulator/mocks/HTTPClient.cpp`

**Step 1: Create WiFi mock header**

Create `simulator/mocks/WiFi.h`:
```cpp
#pragma once

#include "Arduino.h"

typedef enum {
    WL_IDLE_STATUS = 0,
    WL_NO_SSID_AVAIL,
    WL_CONNECTED,
    WL_CONNECT_FAILED,
    WL_CONNECTION_LOST,
    WL_DISCONNECTED
} wl_status_t;

class IPAddress {
public:
    IPAddress() : addr{192, 168, 1, 100} {}
    IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) : addr{a, b, c, d} {}

    String toString() const {
        return String(addr[0]) + "." + String(addr[1]) + "." +
               String(addr[2]) + "." + String(addr[3]);
    }

private:
    uint8_t addr[4];
};

class WiFiClass {
public:
    void mode(uint8_t mode) {}
    void setHostname(const char* hostname) {}
    void begin(const char* ssid, const char* password);
    wl_status_t status();
    IPAddress localIP();
    void reconnect();
    bool softAP(const char* ssid, const char* password);

private:
    wl_status_t currentStatus = WL_DISCONNECTED;
};

extern WiFiClass WiFi;

#define WIFI_AP_STA 3
#define WIFI_AP 2
#define WIFI_STA 1
```

**Step 2: Implement WiFi mock**

Create `simulator/mocks/WiFi.cpp`:
```cpp
// ABOUTME: WiFi mock for simulator
// ABOUTME: Simulates WiFi connection state without actual network operations

#include "WiFi.h"
#include <cstdlib>

WiFiClass WiFi;

void WiFiClass::begin(const char* ssid, const char* password) {
    const char* mockMode = getenv("SIMULATOR_MOCK");
    if (mockMode && std::string(mockMode) == "1") {
        currentStatus = WL_CONNECTED;
        Serial.println("WiFi connected (mock mode)");
    } else {
        // In real mode, we don't actually connect to WiFi
        // but pretend we're connected for HTTP operations
        currentStatus = WL_CONNECTED;
        Serial.println("WiFi connected (simulator mode)");
    }
}

wl_status_t WiFiClass::status() {
    return currentStatus;
}

IPAddress WiFiClass::localIP() {
    return IPAddress(192, 168, 1, 100);
}

void WiFiClass::reconnect() {
    currentStatus = WL_CONNECTED;
}

bool WiFiClass::softAP(const char* ssid, const char* password) {
    Serial.print("Created AP: ");
    Serial.println(ssid);
    return true;
}
```

**Step 3: Create HTTPClient mock header**

Create `simulator/mocks/HTTPClient.h`:
```cpp
#pragma once

#include "Arduino.h"

#define HTTP_GET 0
#define HTTP_POST 1

class HTTPClient {
public:
    HTTPClient();
    ~HTTPClient();

    bool begin(const String& url);
    void addHeader(const String& name, const String& value);
    int GET();
    int POST(const String& payload);
    String getString();
    void end();

private:
    String currentUrl;
    String responseBody;
    int responseCode;
    bool useMockMode;

    bool loadMockResponse();
};
```

**Step 4: Implement HTTPClient mock (basic)**

Create `simulator/mocks/HTTPClient.cpp`:
```cpp
// ABOUTME: HTTPClient mock for simulator
// ABOUTME: Routes requests to mock JSON files or real libcurl (future)

#include "HTTPClient.h"
#include <cstdlib>
#include <fstream>
#include <sstream>

HTTPClient::HTTPClient() : responseCode(0), useMockMode(true) {
    const char* mockMode = getenv("SIMULATOR_MOCK");
    if (mockMode && std::string(mockMode) == "0") {
        useMockMode = false;
    }
}

HTTPClient::~HTTPClient() {
    end();
}

bool HTTPClient::begin(const String& url) {
    currentUrl = url;
    return true;
}

void HTTPClient::addHeader(const String& name, const String& value) {
    // Store headers if needed
}

int HTTPClient::GET() {
    if (useMockMode) {
        if (loadMockResponse()) {
            return responseCode;
        }
        return 404;
    } else {
        // TODO: Implement real HTTP via libcurl
        Serial.println("Real HTTP not yet implemented");
        return 500;
    }
}

int HTTPClient::POST(const String& payload) {
    if (useMockMode) {
        if (loadMockResponse()) {
            return responseCode;
        }
        return 404;
    } else {
        // TODO: Implement real HTTP via libcurl
        Serial.println("Real HTTP not yet implemented");
        return 500;
    }
}

String HTTPClient::getString() {
    return responseBody;
}

void HTTPClient::end() {
    responseBody = "";
    responseCode = 0;
}

bool HTTPClient::loadMockResponse() {
    // Determine mock file from URL
    std::string mockFile;

    if (currentUrl.find("oauth2.googleapis.com/token") != std::string::npos) {
        mockFile = "simulator/mock_responses/oauth/token_success.json";
    } else if (currentUrl.find("calendar/v3/calendars") != std::string::npos) {
        if (currentUrl.find("/events") != std::string::npos) {
            mockFile = "simulator/mock_responses/calendar/events_none.json";
        } else {
            mockFile = "simulator/mock_responses/calendar/calendars_list.json";
        }
    } else {
        Serial.print("No mock response for URL: ");
        Serial.println(currentUrl);
        return false;
    }

    std::ifstream file(mockFile);
    if (!file.is_open()) {
        Serial.print("Mock file not found: ");
        Serial.println(mockFile);
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    responseBody = buffer.str();
    responseCode = 200;

    return true;
}
```

**Step 5: Create placeholder mock responses**

```bash
mkdir -p simulator/mock_responses/oauth
mkdir -p simulator/mock_responses/calendar
```

Create `simulator/mock_responses/oauth/token_success.json`:
```json
{
  "access_token": "mock_access_token_12345",
  "expires_in": 3600,
  "refresh_token": "mock_refresh_token_67890",
  "token_type": "Bearer"
}
```

Create `simulator/mock_responses/calendar/events_none.json`:
```json
{
  "items": []
}
```

Create `simulator/mock_responses/calendar/calendars_list.json`:
```json
{
  "items": [
    {
      "id": "primary",
      "summary": "Test Calendar"
    }
  ]
}
```

**Step 6: Update Makefile**

Update SRCS:
```makefile
SRCS = main.cpp simulated_time.cpp terminal_display.cpp \
       mocks/Arduino.cpp mocks/Preferences.cpp \
       mocks/WiFi.cpp mocks/HTTPClient.cpp \
       mocks/freertos/queue.cpp mocks/freertos/task.cpp
```

**Step 7: Build**

```bash
make simulator
```

Expected: Compiles successfully

**Step 8: Commit**

```bash
git add simulator/mocks/WiFi.h simulator/mocks/WiFi.cpp \
        simulator/mocks/HTTPClient.h simulator/mocks/HTTPClient.cpp \
        simulator/mock_responses/ \
        simulator/Makefile
git commit -m "feat: add WiFi and HTTPClient mocks with JSON responses"
```

---

## Phase 4: Remaining Mocks and Firmware Integration

### Task 9: Add Remaining ESP32 Dependencies

**Files:**
- Create: `simulator/mocks/WebServer.h`
- Create: `simulator/mocks/WebServer.cpp`
- Create: `simulator/mocks/ArduinoJson.h`
- Create: `simulator/mocks/ESPmDNS.h`
- Create: `simulator/mocks/ESP.h`

**Step 1: Create WebServer mock**

Create `simulator/mocks/WebServer.h`:
```cpp
#pragma once

#include "Arduino.h"
#include <functional>

typedef std::function<void(void)> THandlerFunction;

class WebServer {
public:
    WebServer(int port) : port(port) {}

    void begin() {
        Serial.print("WebServer started on port ");
        Serial.println(port);
    }

    void handleClient() {
        // No-op for simulator
    }

    void on(const String& uri, THandlerFunction handler) {
        // Store handler
    }

    void on(const String& uri, int method, THandlerFunction handler) {
        // Store handler
    }

    bool hasArg(const String& name) { return false; }
    String arg(const String& name) { return ""; }
    void send(int code, const String& contentType, const String& content) {
        Serial.println(content);
    }
    void sendHeader(const String& name, const String& value) {}

private:
    int port;
};
```

**Step 2: Create ArduinoJson mock**

Create `simulator/mocks/ArduinoJson.h`:
```cpp
#pragma once

#include "Arduino.h"
#include <map>
#include <vector>

// Minimal ArduinoJson mock - just enough for our firmware
class JsonVariant {
public:
    JsonVariant() : strValue(""), intValue(0) {}

    template<typename T>
    T as() const;

    const char* as<const char*>() const { return strValue.c_str(); }
    String as<String>() const { return strValue; }
    int as<int>() const { return intValue; }

    void setString(const String& s) { strValue = s; }
    void setInt(int i) { intValue = i; }

    String operator[](const char* key) const {
        return strValue;
    }

private:
    String strValue;
    int intValue;
};

class JsonArray {
public:
    class iterator {
    public:
        iterator(size_t idx, JsonArray* arr) : index(idx), array(arr) {}
        bool operator!=(const iterator& other) const { return index != other.index; }
        iterator& operator++() { ++index; return *this; }
        JsonVariant operator*() const { return array->items[index]; }
    private:
        size_t index;
        JsonArray* array;
    };

    iterator begin() { return iterator(0, this); }
    iterator end() { return iterator(items.size(), this); }

    void add(const JsonVariant& item) { items.push_back(item); }
    size_t size() const { return items.size(); }

private:
    std::vector<JsonVariant> items;
    friend class iterator;
};

class JsonDocument {
public:
    JsonVariant operator[](const char* key) {
        return data[key];
    }

    JsonArray createNestedArray(const char* key) {
        return JsonArray();
    }

    JsonArray as<JsonArray>() {
        return JsonArray();
    }

private:
    std::map<String, JsonVariant> data;
};

class DynamicJsonDocument : public JsonDocument {
public:
    DynamicJsonDocument(size_t capacity) {}
};

template<size_t capacity>
class StaticJsonDocument : public JsonDocument {
public:
    StaticJsonDocument() {}
};

enum class DeserializationError {
    Ok,
    NoMemory,
    InvalidInput
};

inline DeserializationError deserializeJson(JsonDocument& doc, const String& input) {
    // Simple mock - doesn't actually parse JSON
    return DeserializationError::Ok;
}

inline void serializeJson(const JsonDocument& doc, String& output) {
    output = "{}";
}
```

**Step 3: Create ESPmDNS mock**

Create `simulator/mocks/ESPmDNS.h`:
```cpp
#pragma once

#include "Arduino.h"

class MDNSClass {
public:
    bool begin(const char* hostname) {
        Serial.print("mDNS started: ");
        Serial.print(hostname);
        Serial.println(".local");
        return true;
    }

    void addService(const char* service, const char* proto, uint16_t port) {
        Serial.print("mDNS service added: ");
        Serial.print(service);
        Serial.print(".");
        Serial.println(proto);
    }
};

extern MDNSClass MDNS;
```

Create `simulator/mocks/ESPmDNS.cpp`:
```cpp
#include "ESPmDNS.h"

MDNSClass MDNS;
```

**Step 4: Create ESP mock**

Create `simulator/mocks/ESP.h`:
```cpp
#pragma once

#include "Arduino.h"
#include <cstdlib>

class ESPClass {
public:
    void restart() {
        Serial.println("ESP restarting...");
        exit(0);
    }
};

extern ESPClass ESP;
```

Create `simulator/mocks/ESP.cpp`:
```cpp
#include "ESP.h"

ESPClass ESP;
```

**Step 5: Update Makefile**

Update SRCS:
```makefile
SRCS = main.cpp simulated_time.cpp terminal_display.cpp \
       mocks/Arduino.cpp mocks/Preferences.cpp \
       mocks/WiFi.cpp mocks/HTTPClient.cpp \
       mocks/ESPmDNS.cpp mocks/ESP.cpp \
       mocks/freertos/queue.cpp mocks/freertos/task.cpp
```

**Step 6: Build**

```bash
make simulator
```

Expected: Compiles successfully

**Step 7: Commit**

```bash
git add simulator/mocks/*.h simulator/mocks/*.cpp simulator/Makefile
git commit -m "feat: add WebServer, ArduinoJson, mDNS, and ESP mocks"
```

---

### Task 10: Add Display Handler Mock

**Files:**
- Create: `simulator/mocks/DisplayHandler.h`
- Create: `simulator/mocks/DisplayHandler.cpp`

**Step 1: Create DisplayHandler mock header**

Create `simulator/mocks/DisplayHandler.h`:
```cpp
#pragma once

#include "Arduino.h"

// Mock LED matrix class
class Adafruit_NeoMatrix {
public:
    void begin() {}
    void clear() {}
    void show() {}
    void setPixelColor(uint16_t n, uint32_t c) {}
    void setPixelColor(uint16_t x, uint16_t y, uint32_t c) {}
    uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
        return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }
};

class DisplayHandler {
public:
    Adafruit_NeoMatrix matrix;

    void begin() {
        Serial.println("Display initialized");
    }
};
```

**Step 2: Create empty implementation**

Create `simulator/mocks/DisplayHandler.cpp`:
```cpp
// ABOUTME: Display handler mock for simulator
// ABOUTME: Terminal rendering of LED matrix happens in simulator/terminal_display.cpp

#include "DisplayHandler.h"

// No implementation needed - all inline
```

**Step 3: Update Makefile**

Update SRCS (add DisplayHandler.cpp):
```makefile
SRCS = main.cpp simulated_time.cpp terminal_display.cpp \
       mocks/Arduino.cpp mocks/Preferences.cpp \
       mocks/WiFi.cpp mocks/HTTPClient.cpp \
       mocks/ESPmDNS.cpp mocks/ESP.cpp mocks/DisplayHandler.cpp \
       mocks/freertos/queue.cpp mocks/freertos/task.cpp
```

**Step 4: Build**

```bash
make simulator
```

Expected: Compiles successfully

**Step 5: Commit**

```bash
git add simulator/mocks/DisplayHandler.h simulator/mocks/DisplayHandler.cpp simulator/Makefile
git commit -m "feat: add DisplayHandler mock for LED matrix"
```

---

## Phase 5: Firmware Integration

### Task 11: Compile First Firmware File

**Files:**
- Modify: `simulator/Makefile`
- Test compile: `bindicator.cpp`

**Step 1: Update Makefile to include firmware sources**

Update `simulator/Makefile`:
```makefile
CXX = g++
CXXFLAGS = -std=c++14 -Wall -DSIMULATOR -DTESTING
CXXFLAGS += -Imocks -Imocks/freertos -I..
LDFLAGS = -pthread -lcurl

TARGET = simulator

# Simulator sources
SIM_SRCS = main.cpp simulated_time.cpp terminal_display.cpp

# Mock sources
MOCK_SRCS = mocks/Arduino.cpp mocks/Preferences.cpp \
            mocks/WiFi.cpp mocks/HTTPClient.cpp \
            mocks/ESPmDNS.cpp mocks/ESP.cpp mocks/DisplayHandler.cpp \
            mocks/freertos/queue.cpp mocks/freertos/task.cpp

# Firmware sources (relative to parent directory)
FIRMWARE_SRCS = ../bindicator.cpp ../config_manager.cpp

SRCS = $(SIM_SRCS) $(MOCK_SRCS) $(FIRMWARE_SRCS)
OBJS = $(SIM_SRCS:.cpp=.o) $(MOCK_SRCS:.cpp=.o)
FIRMWARE_OBJS = $(notdir $(FIRMWARE_SRCS:.cpp=.o))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS) $(FIRMWARE_OBJS)
	$(CXX) $(OBJS) $(FIRMWARE_OBJS) -o $(TARGET) $(LDFLAGS)

# Simulator and mock objects
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Firmware objects (compile from parent directory)
bindicator.o: ../bindicator.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

config_manager.o: ../config_manager.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(FIRMWARE_OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)
```

**Step 2: Try to build**

```bash
make simulator
```

Expected: Compilation errors for missing dependencies

**Step 3: Document errors**

Note which headers/files are missing and add to next task list.
Common issues:
- Missing time.h includes
- Missing function declarations
- Circular dependencies

**Step 4: Create compilation success marker**

Once it compiles (may need next task), commit:
```bash
git add simulator/Makefile
git commit -m "feat: integrate bindicator.cpp and config_manager.cpp into simulator"
```

---

### Task 12: Add Remaining Firmware Files

**Files:**
- Modify: `simulator/Makefile`
- Add: All remaining firmware .cpp files

**Step 1: Add all firmware sources to Makefile**

Update `FIRMWARE_SRCS` in `simulator/Makefile`:
```makefile
FIRMWARE_SRCS = ../bindicator.cpp ../config_manager.cpp \
                ../oauth_handler.cpp ../calendar_handler.cpp \
                ../tasks.cpp ../time_manager.cpp \
                ../utils.cpp ../button_handler.cpp \
                ../serial_commands.cpp ../setup_server.cpp \
                ../animations.cpp
```

**Step 2: Add build rules for new firmware objects**

Add after existing firmware object rules:
```makefile
oauth_handler.o: ../oauth_handler.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

calendar_handler.o: ../calendar_handler.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

tasks.o: ../tasks.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

time_manager.o: ../time_manager.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

utils.o: ../utils.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

button_handler.o: ../button_handler.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

serial_commands.o: ../serial_commands.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

setup_server.o: ../setup_server.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

animations.o: ../animations.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
```

**Step 3: Fix any remaining compilation errors**

Common fixes needed:
- Add `#ifdef SIMULATOR` guards for hardware-specific code
- Mock missing functions
- Add missing includes

**Step 4: Build**

```bash
make simulator
```

Expected: All firmware files compile successfully

**Step 5: Commit**

```bash
git add simulator/Makefile
git commit -m "feat: integrate all firmware source files into simulator"
```

---

### Task 13: Wire Up Firmware Setup and Loop

**Files:**
- Modify: `simulator/main.cpp`

**Step 1: Add firmware function declarations**

Add to top of `simulator/main.cpp`:
```cpp
// Firmware entry points
extern void setup();
extern void loop();

// Firmware globals (need to be declared)
extern QueueHandle_t commandQueue;
```

**Step 2: Add firmware initialization to main**

Modify main() function:
```cpp
int main(int argc, char** argv) {
    std::cout << "Bindicator Simulator v0.1" << std::endl;
    std::cout << "Initializing firmware..." << std::endl;

    // Run firmware setup()
    setup();

    std::cout << "\nFirmware initialized. Type 'help' for commands" << std::endl;

    // TODO: Start loop() in background thread

    std::string command;
    while (std::getline(std::cin, command)) {
        if (command == "quit" || command == "exit") {
            break;
        } else if (command == "help") {
            std::cout << "Available commands:" << std::endl;
            std::cout << "  help  - Show this message" << std::endl;
            std::cout << "  state - Show current device state" << std::endl;
            std::cout << "  quit  - Exit simulator" << std::endl;
        } else if (command == "state") {
            std::cout << "Time: " << millis() << "ms" << std::endl;
            // TODO: Add more state inspection
        } else {
            std::cout << "Unknown command: " << command << std::endl;
        }
    }

    return 0;
}
```

**Step 3: Build and test**

```bash
make simulator
./simulator/simulator
```

Expected: Firmware setup() runs, simulator accepts commands

**Step 4: Commit**

```bash
git add simulator/main.cpp
git commit -m "feat: wire up firmware setup() and interactive command loop"
```

---

## Phase 6: Interactive Runtime

### Task 14: Add Background Firmware Loop Thread

**Files:**
- Modify: `simulator/main.cpp`

**Step 1: Add firmware loop thread**

Modify `simulator/main.cpp`:
```cpp
#include <pthread.h>
#include <atomic>

static std::atomic<bool> firmwareRunning(true);

static void* firmwareLoopThread(void* arg) {
    while (firmwareRunning) {
        loop();
        usleep(10000);  // 10ms delay
    }
    return nullptr;
}

int main(int argc, char** argv) {
    std::cout << "Bindicator Simulator v0.1" << std::endl;
    std::cout << "Initializing firmware..." << std::endl;

    // Run firmware setup()
    setup();

    std::cout << "\nFirmware initialized. Type 'help' for commands" << std::endl;

    // Start firmware loop in background
    pthread_t loopThread;
    pthread_create(&loopThread, nullptr, firmwareLoopThread, nullptr);

    std::string command;
    while (std::getline(std::cin, command)) {
        if (command == "quit" || command == "exit") {
            break;
        } else if (command == "help") {
            std::cout << "Available commands:" << std::endl;
            std::cout << "  help   - Show this message" << std::endl;
            std::cout << "  state  - Show current device state" << std::endl;
            std::cout << "  press  - Simulate button press" << std::endl;
            std::cout << "  jump <time> - Jump forward in time (e.g., 'jump +1h')" << std::endl;
            std::cout << "  quit   - Exit simulator" << std::endl;
        } else if (command == "state") {
            std::cout << "Time: " << millis() << "ms" << std::endl;
        } else if (command == "press") {
            std::cout << "Button press simulated" << std::endl;
            // TODO: Trigger button handler
        } else if (command.substr(0, 4) == "jump") {
            // TODO: Implement time jumping
            std::cout << "Time jumping not yet implemented" << std::endl;
        } else {
            std::cout << "Unknown command: " << command << std::endl;
        }
    }

    // Stop firmware loop
    firmwareRunning = false;
    pthread_join(loopThread, nullptr);

    return 0;
}
```

**Step 2: Build and test**

```bash
make simulator
./simulator/simulator
# Firmware should run in background
# Type "help" then "quit"
```

Expected: Firmware runs continuously while accepting commands

**Step 3: Commit**

```bash
git add simulator/main.cpp
git commit -m "feat: run firmware loop() in background thread"
```

---

## Completion Criteria

After completing all tasks:

1. Simulator compiles without errors
2. Firmware setup() and loop() execute
3. Terminal displays capabilities correctly
4. Commands can be entered interactively
5. Mock responses load from JSON files
6. Time simulation works

## Testing Strategy

**Unit Tests**: Each mock should have standalone test (already done for queue, time)
**Integration Test**: Run simulator and verify:
- setup() completes without crash
- loop() runs without crash
- Commands are accepted
- State persists to simulator-state.json

**Manual Testing Checklist**:
- [ ] `make simulator` builds successfully
- [ ] `./simulator/simulator` starts without errors
- [ ] `help` command shows available commands
- [ ] `state` command shows current time
- [ ] `quit` command exits cleanly
- [ ] simulator-state.json created after Preferences operations

## Known Limitations

As documented in design:
- pthread scheduling differs from FreeRTOS
- Terminal rendering is approximate
- Network mock mode only (real HTTP deferred to Phase 2)
- No actual hardware peripherals

## Next Steps After Phase 1

1. Implement time control commands (jump, speed)
2. Add button simulation
3. Implement real HTTP with libcurl
4. Add display rendering with RGB visualization
5. Create integration tests
6. Add mock response scenarios for error testing
