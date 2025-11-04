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

## Known Limitations & Technical Considerations

### FreeRTOS Threading Model

The simulator uses pthreads to emulate FreeRTOS tasks. Known scheduling differences:

**Priority Handling**
- FreeRTOS: Strict priority preemption (higher priority always runs)
- Pthreads: OS-level scheduling, priorities are hints not guarantees
- **Impact**: High-priority animation task may not always preempt calendar task
- **Mitigation**: Use condition variables for critical synchronization, not priority alone

**Task Pinning**
- FreeRTOS: Tasks pinned to specific cores (animation on Core 1, calendar on Core 0)
- Pthreads: OS decides thread placement, no guaranteed core affinity on macOS
- **Impact**: Concurrent execution patterns may differ slightly
- **Mitigation**: Use proper synchronization primitives (mutexes, condition variables) rather than relying on task pinning

**Queue Behaviour**
- FreeRTOS: Fixed-size queues with blocking/non-blocking send/receive
- Pthread implementation: Mutex-protected deque with condition variables
- **Expected differences**: Timing of queue full/empty conditions under high load
- **Mitigation**: Prototype early and document actual behaviour vs ESP-IDF

**Time Slicing**
- FreeRTOS: Configurable tick rate (typically 1ms on ESP32)
- Pthreads: OS time slicing (macOS uses ~10ms quanta)
- **Impact**: Tight timing loops may behave differently
- **Mitigation**: Don't rely on sub-10ms timing precision in tests

**Testing Strategy**: Integration tests should focus on functional correctness (state transitions, API interactions) rather than precise timing or priority behaviour.

### Display Rendering

**Terminal Requirements**
- UTF-8 locale support (for Unicode block characters: █ ▀ ▄)
- ANSI color code support (ESC sequences)
- Minimum 80x24 terminal size
- Monospace font required for proper alignment

**Terminal Detection**
The simulator will detect capabilities at startup:
```cpp
bool detectTerminalCapabilities() {
    // Check TERM environment variable
    // Test for UTF-8 support
    // Verify ANSI color codes work
    // Measure terminal dimensions
}
```

**Graceful Degradation**
- No UTF-8: Fall back to ASCII characters (#, *, @)
- No colors: Use brightness indicators (█ = full, ▓ = medium, ▒ = low)
- Small terminal: Display state text-only without matrix
- Warnings printed to stderr if features unavailable

**Visual Fidelity**
- Terminal rendering is approximate, not pixel-perfect
- Animation timing may differ from hardware refresh rate
- Colors may not match physical LEDs exactly

### Other Hardware Differences

- Network timing differs from ESP32 WiFi chip
- No actual hardware peripherals (SPI, I2C, GPIO)
- Memory allocation patterns differ (heap vs PSRAM)
- Flash writes (Preferences) are file I/O, not NVS flash

These limitations don't affect business logic testing, which is the primary goal.

## Credential Management

### OAuth Secrets

The simulator needs Google OAuth credentials (client ID and secret) to authenticate with Google Calendar API in real-network mode.

**Development Environment**
Credentials loaded from environment variables or `.env` file (git-ignored):
```bash
# .env (not committed)
GOOGLE_CLIENT_ID=xxxx.apps.googleusercontent.com
GOOGLE_CLIENT_SECRET=xxxx
GOOGLE_REDIRECT_URI=https://your-cloud-run-url/callback
```

Simulator reads these on startup:
```cpp
const char* clientId = getenv("GOOGLE_CLIENT_ID");
const char* clientSecret = getenv("GOOGLE_CLIENT_SECRET");
```

If not set, simulator warns and falls back to mock mode automatically.

**CI/CD Environment**
GitHub Actions secrets injected as environment variables:
```yaml
- name: Run simulator integration tests
  env:
    GOOGLE_CLIENT_ID: ${{ secrets.GOOGLE_CLIENT_ID }}
    GOOGLE_CLIENT_SECRET: ${{ secrets.GOOGLE_CLIENT_SECRET }}
  run: |
    make simulator
    ./tests/integration/run_all.sh
```

**Mock Mode (Default for CI)**
Most CI tests should use `--mock` mode to avoid:
- Credential leakage risk
- API rate limits
- Network flakiness
- Requiring real OAuth setup

Only a small subset of integration tests should exercise real OAuth flow, and these should:
- Use a dedicated test Google account
- Run in a protected CI environment (not on forks)
- Store refresh token in CI secrets (not client secret)

**Security Best Practices**
- Never commit credentials to git
- `.env` file in `.gitignore`
- `simulator-state.json` (contains refresh token) in `.gitignore`
- Use dedicated OAuth client for simulator (not production credentials)
- Rotate credentials if accidentally leaked

### WiFi Credentials

Not needed for simulator (no actual WiFi connection). ConfigManager stores them in `simulator-state.json` but they're not used for network operations.

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

## Implementation Approach

### Phase 1: Core Infrastructure (Prototype Critical Components)

**1. FreeRTOS Queue Implementation**
Build and validate pthread-based queue early to understand behaviour under load:
```cpp
// simulator/mocks/freertos/queue_test.cpp
// Test: Multiple producers, single consumer
// Test: Queue full/empty conditions
// Test: Timeout behaviour
// Document: Observed differences from ESP-IDF
```

Load testing scenarios:
- 10 producers sending to same queue
- Queue full conditions with `xQueueSend(q, data, 0)` (non-blocking)
- Queue full conditions with `xQueueSend(q, data, portMAX_DELAY)` (blocking)
- Measure timing differences vs expected FreeRTOS behaviour

**Output**: Documented queue behaviour and any known deviations to inform test expectations.

**2. Terminal Capability Detection**
Implement early to avoid confusing rendering issues:
```cpp
// simulator/terminal_display.cpp
TerminalCapabilities detectCapabilities() {
    // Check TERM, COLORTERM environment variables
    // Test UTF-8 by writing test character
    // Probe for 256-color support
    // Detect terminal size
}
```

**3. Time Simulation Core**
Basic time control without full firmware integration:
```cpp
// simulator/simulator_config.cpp
class SimulatedTime {
    unsigned long baseTime;
    float multiplier;
    unsigned long millis();
    void advance(unsigned long ms);
    void setMultiplier(float x);
};
```

### Phase 2: Mock Layer Completion

1. Arduino core APIs (millis, delay, Serial)
2. WiFi mock with mode switching
3. HTTPClient mock with canned responses
4. Preferences with JSON backend
5. DisplayHandler with terminal renderer

Each mock should have standalone test before integration.

### Phase 3: Firmware Integration

1. Compile first firmware file (bindicator.cpp) with mocks
2. Add files incrementally: config_manager, oauth_handler, calendar_handler
3. Validate each addition compiles and links
4. Run firmware `setup()` and `loop()` functions

### Phase 4: Interactive Runtime

1. Command parser and stdin handling
2. Display rendering loop
3. Time control commands
4. State inspection commands

### Phase 5: Testing & Validation

1. Create mock API responses for common scenarios
2. Write integration tests
3. Validate critical bugs can be reproduced (millis overflow, state desync)
4. Document remaining limitations

### Risk Mitigation

**Risk: pthread queue behaviour differs significantly**
- Mitigation: Prototype in Phase 1, document differences, adjust test expectations
- Fallback: Use single-threaded event loop if threading proves problematic

**Risk: Terminal rendering issues across platforms**
- Mitigation: Detect capabilities early, graceful degradation
- Fallback: Text-only mode with no graphics

**Risk: libcurl integration complexity**
- Mitigation: Start with mock-only mode, add real network later
- Fallback: Stay mock-only if real HTTP proves difficult

## Future Enhancements (Not in Scope)

- SDL2 graphical window for pixel-perfect LED rendering
- Web interface for remote access/demonstration
- Recording/playback of interaction sequences
- Performance profiling and optimization tools
