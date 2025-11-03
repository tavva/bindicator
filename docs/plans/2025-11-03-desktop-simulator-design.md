# Desktop Simulator Design

**Date**: 2025-11-03
**Status**: Approved
**Purpose**: Enable development, debugging, and testing of Bindicator firmware on desktop without hardware

## Overview

The desktop simulator runs actual firmware source code (bindicator.cpp, oauth_handler.cpp, tasks.cpp, etc.) on macOS/Linux by providing a comprehensive mock layer for ESP32/Arduino APIs. This enables rapid iteration, debugging with standard tools, and automated integration testing.

## Goals

1. **Development & Debugging** - Test full application flow, debug issues (millis overflow, state synchronization), iterate quickly without hardware uploads
2. **Automated Testing** - Run integration tests in CI/CD, test state machine transitions over simulated time, validate OAuth and calendar logic with mock responses
3. **Visual Demonstration** - Show device behavior without hardware, create recordings/screenshots of different states

## Architecture

### High-Level Structure

```
┌─────────────────────────────────────────┐
│  Firmware Source (unchanged)            │
│  - bindicator.cpp                       │
│  - oauth_handler.cpp                    │
│  - calendar_handler.cpp                 │
│  - tasks.cpp                            │
│  - config_manager.cpp                   │
└──────────────┬──────────────────────────┘
               │ calls
┌──────────────▼──────────────────────────┐
│  Mock Layer (simulator-specific)        │
│  - Arduino.h/cpp (millis, delay, etc.)  │
│  - WiFi/HTTPClient (real or mock)       │
│  - DisplayHandler (terminal renderer)   │
│  - FreeRTOS (pthreads implementation)   │
│  - Preferences (JSON file backend)      │
│  - WebServer (basic HTTP for setup)     │
└──────────────┬──────────────────────────┘
               │
┌──────────────▼──────────────────────────┐
│  Simulator Runtime                      │
│  - Main loop with stdin command parser  │
│  - Terminal UI renderer                 │
│  - Time control (real/accelerated/jump) │
│  - Network mode switcher (real/mock)    │
└─────────────────────────────────────────┘
```

**Key principle:** Firmware code doesn't know it's running in a simulator. All hardware differences are abstracted through existing Arduino/ESP32 APIs.

## Components

### 1. Mock Layer

Provides ESP32/Arduino APIs that work on desktop, expanding existing `tests/mocks/` infrastructure.

**Time & Timing (Arduino.h/cpp)**
- `millis()` - Returns simulated time
- `delay()` - Advances simulated time
- `setSimulatedTime()` - Jump to specific time
- `setTimeMultiplier()` - Accelerate time (e.g., 100x)

**WiFi/Network (WiFi.h, HTTPClient.h)**
- Real mode: Uses system network stack via libcurl
- Mock mode: Returns canned responses from JSON files
- Supports testing error scenarios (401, 429, 500) without breaking real OAuth

**Display (DisplayHandler)**
- Terminal renderer using ANSI colors and Unicode blocks (█)
- Renders 8x8 RGB buffer to terminal
- Updates in place for animation effects

**FreeRTOS (pthreads wrapper)**
- `xTaskCreatePinnedToCore()` → `pthread_create()`
- `xQueueCreate()` → pthread mutex + condition variable
- `vTaskDelay()` → `usleep()` with simulated time

**Preferences (JSON file backend)**
- Reads/writes to `simulator-state.json`
- Ephemeral mode: In-memory map, no persistence
- Enables hand-editing state for testing scenarios

### 2. Simulator Runtime

**Main Loop**
```cpp
int main(int argc, char** argv) {
    // Parse args: --mock, --ephemeral, --time-multiplier=100
    SimulatorConfig config = parseArgs(argc, argv);

    // Initialize mocks with config
    initializeMocks(config);

    // Call firmware setup()
    setup();

    // Start background thread running firmware loop()
    pthread_create(&loopThread, NULL, firmwareLoopThread, NULL);

    // Interactive command loop
    while (running) {
        renderDisplay();        // Show 8x8 LED matrix
        printStatus();          // Show state, time, network status
        handleCommand();        // Process stdin commands
        usleep(50000);         // 50ms refresh
    }
}
```

**Command Interface (stdin)**
```
press              - Simulate short button press
long_press         - Simulate long button press (3s hold)
jump <time>        - Jump forward in time (e.g., "jump +1h", "jump +2d")
set_time <HH:MM>   - Set time to specific hour (e.g., "set_time 03:00")
speed <multiplier> - Set time acceleration (e.g., "speed 100")
trigger_calendar   - Force calendar check now
mock <scenario>    - Load mock response set (e.g., "mock error_401")
state              - Show current device state
prefs              - Show all preferences
reset              - Clear all preferences
quit               - Exit simulator
```

**Display Output**
```
┌─────────────────────────────────────────┐
│ Bindicator Simulator                    │
│ Time: 2024-11-03 14:30:45 (100x speed) │
│ State: RECYCLING_DUE                    │
│ WiFi: Connected (mock mode)             │
├─────────────────────────────────────────┤
│ 🟦🟦🟦🟦🟦🟦🟦🟦                        │
│ 🟦🟩🟩🟩🟩🟩🟩🟦                        │
│ 🟦🟩⬜⬜⬜⬜🟩🟦                        │
│ 🟦🟩🟩🟩🟩🟩🟩🟦                        │
│ 🟦🟩🟩🟩🟩🟩🟩🟦                        │
│ 🟦🟩🟩🟩🟩🟩🟩🟦                        │
│ 🟦🟩🟩🟩🟩🟩🟩🟦                        │
│ 🟦🟦🟦🟦🟦🟦🟦🟦                        │
└─────────────────────────────────────────┘
> _
```

### 3. Network & API Mocking

**Mock Response Structure**
```
simulator/
  mock_responses/
    oauth/
      token_success.json          # Valid token exchange
      token_expired.json          # Expired token response
      token_error_401.json        # Unauthorized error
    calendar/
      events_recycling.json       # Calendar with recycling event today
      events_rubbish.json         # Calendar with rubbish event today
      events_both.json            # Both bins due
      events_none.json            # No events today
      events_error_404.json       # Calendar not found
      calendars_list.json         # Available calendars response
```

**HTTPClient Implementation**
- Default: Real network calls to Google APIs
- Mock mode (--mock flag): Returns canned responses from JSON files
- Runtime switchable: `mock real` / `mock auto` commands
- Enables testing error scenarios and offline development

### 4. Time Control

Time-dependent behaviors:
- Calendar checks every hour
- 3am daily reset when bin is completed
- 5-minute error retry intervals
- Token expiry tracking
- Millis overflow (49 days)

**Time Modes**
- Real-time: Time passes normally (default)
- Accelerated: Configurable multiplier (e.g., 100x = 1 hour in 36 seconds)
- Manual jump: Skip to specific time or offset (e.g., `jump +49d` to test overflow)
- Set absolute: Jump to specific hour (e.g., `set_time 03:00` to test reset)

### 5. State Persistence

**Default: JSON file (`simulator-state.json`)**
- OAuth tokens persist between runs
- Human-readable and hand-editable
- Git-ignored
- Enables setting up specific test scenarios

**Ephemeral mode (`--ephemeral`)**
- In-memory storage only
- Clean slate every run
- For automated testing

**Commands**
- `--reset` flag deletes JSON file
- `reset` command clears all preferences at runtime

## File Structure

```
bindicator/
  *.cpp, *.h, *.ino           # Existing firmware (unchanged)
  Makefile                     # Enhanced with simulator targets
  simulator/
    main.cpp                   # Simulator entry point & command loop
    simulator_config.h/cpp     # Config parsing, time control
    terminal_display.h/cpp     # LED matrix terminal renderer
    mocks/
      Arduino.h/cpp            # Enhanced from tests/mocks
      WiFi.h/cpp               # Network mock/real switcher
      HTTPClient.h/cpp         # HTTP mock/real switcher
      WebServer.h/cpp          # Basic HTTP server for setup mode
      DisplayHandler.h/cpp     # Terminal display implementation
      freertos/                # pthread-based FreeRTOS
        FreeRTOS.h
        task.h
        queue.h
      Preferences.h/cpp        # JSON file backend
    mock_responses/
      oauth/
      calendar/
  tests/                       # Existing unit tests (unchanged)
```

## Build System

**Makefile additions:**
```makefile
SIM_CXX = g++
SIM_FLAGS = -std=c++14 -Wall -DSIMULATOR -Isimulator/mocks -Isimulator
SIM_LIBS = -pthread -lcurl

simulator: simulator/main.o simulator/*.o *.cpp
	$(SIM_CXX) $(SIM_FLAGS) -o simulator $^ $(SIM_LIBS)

sim-run: simulator
	./simulator

sim-clean:
	rm -f simulator simulator/*.o simulator/mocks/*.o
```

**Usage:**
```bash
make simulator       # Build simulator binary
make sim-run         # Build and run interactively
./simulator --help   # Show options
```

## Workflows

### Development
```bash
# Quick iteration on firmware logic
make sim-run
> jump +1h              # Test calendar check
> set_time 03:00        # Test 3am reset
> press                 # Test button handling

# Test with real Google Calendar
./simulator             # Uses real OAuth & API calls
# Authenticate once, token persists

# Test edge cases with mocks
./simulator --mock
> mock calendar/events_both.json
> trigger_calendar
```

### Debugging
```bash
# Debug millis overflow bug
./simulator
> jump +49d              # Jump to near overflow
> state                  # Check current state
> trigger_error          # Force error state
> jump +1m               # See if retry logic works

# Attach debugger
lldb ./simulator
(lldb) breakpoint set -n Bindicator::shouldRetryAfterError
(lldb) run
```

### Automated Testing
```bash
# Integration tests using simulator
./tests/integration/test_state_machine.sh
#!/bin/bash
./simulator --ephemeral --mock <<EOF
mock calendar/events_recycling.json
trigger_calendar
press
set_time 03:00
jump +1m
quit
EOF
```

### CI/CD
```yaml
- name: Run simulator integration tests
  run: |
    make simulator
    ./tests/integration/run_all.sh
```

## Testing Capabilities

**Simulated failures:**
```
inject wifi_disconnect     # Simulate WiFi dropping
inject api_error 401       # Simulate OAuth expiry
inject api_error 429       # Simulate rate limiting
inject time_sync_fail      # Simulate NTP failure
inject millis_overflow     # Jump to millis() overflow point
```

**Crash detection:**
- Catches signals (segfault, assertion)
- Prints stack trace
- Shows which task crashed (animation, calendar, button)
- Saves state before exiting

**State validation:**
```
validate                   # Check for:
  - Queue message consistency
  - State machine validity
  - Token expiry logic
  - Preference corruption
```

**Performance testing:**
```
speed 1000                 # 1000x time acceleration
jump +50d                  # Test millis overflow at 49 days
stats                      # Show memory usage, queue depths
```

## Known Limitations

- LED matrix animations won't look identical (terminal vs physical LEDs)
- Network timing differs from ESP32 WiFi chip
- Thread scheduling may differ from FreeRTOS
- No actual hardware peripherals (SPI, I2C)

These limitations don't affect business logic testing, which is the primary goal.

## Success Criteria

1. Simulator can run through complete device lifecycle (setup → normal operation → error recovery)
2. Real OAuth flow works (authenticate, refresh tokens, persist)
3. Mock mode enables testing all error scenarios without network
4. Time control allows testing 3am reset, error retry intervals, millis overflow
5. Terminal display clearly shows device state and LED animations
6. Can attach debugger and step through firmware code
7. Integration tests can run in CI/CD

## Dependencies

- **g++** (C++14 support)
- **pthread** (included with macOS/Linux)
- **libcurl** (for real HTTP requests)
- **Standard libraries** (no external dependencies beyond above)

## Future Enhancements (Not in Scope)

- SDL2 graphical window for pixel-perfect LED rendering
- Web interface for remote access/demonstration
- Recording/playback of interaction sequences
- Performance profiling and optimization tools
