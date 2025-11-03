# AGENTS.md

This file provides guidance to agents such as Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

The Bindicator is an ESP32-based IoT device that displays which bins (rubbish/recycling) need to be put out based on Google Calendar events. It features an LED matrix display, button interface, OAuth authentication with Google Calendar, and a web-based setup interface.

## Hardware Platform

- **Target**: ESP32 (specifically ESP32-S3)
- **Board FQBN**: `esp32:esp32:esp32s3:CDCOnBoot=cdc`
- **Default Port**: `/dev/cu.usbmodem2101`
- **Build System**: Arduino CLI via Makefile

## Development Commands

### Building and Uploading (Arduino)

```bash
make                # Build and upload firmware to device
make compile        # Build only (no upload)
make nocache        # Clean build and upload
make just-upload    # Upload existing build without recompiling
make monitor        # Open serial monitor (auto-reconnects)
make update         # Update Arduino platform packages
```

### Testing (Native C++)

```bash
cd tests
./run_tests.sh      # Build and run all unit tests (requires GoogleTest)
make test           # Alternative: run tests directly
```

Tests require GoogleTest installed via Homebrew: `brew install googletest`

### Cloud Function

The `cloud-run/` directory contains a Google Cloud Function (Node.js) used as the OAuth redirect handler for the device setup process.

## Architecture Overview

### Core State Machine (bindicator.cpp/h)

The `Bindicator` class manages the device's primary state machine with these states:
- `NO_COLLECTION`: No bins due
- `RECYCLING_DUE`: Recycling bin needs to go out
- `RUBBISH_DUE`: Rubbish bin needs to go out
- Error states (WiFi, API, other)
- Setup mode

State transitions are triggered by:
- Calendar checks (periodic polling)
- Button presses (mark bin as taken out)
- Time-based resets (3am daily)
- Error conditions

### Multi-threaded Architecture (FreeRTOS)

The firmware uses FreeRTOS tasks pinned to specific cores:
- **animationTask** (Core 1, Priority 2): Handles LED matrix animations and display updates
- **calendarTask** (Core 0, Priority 1): Polls Google Calendar API periodically

Tasks communicate via a FreeRTOS queue (`commandQueue`) that carries `Command` enums to trigger display changes.

### Key Components

1. **OAuthHandler**: Manages Google OAuth 2.0 flow, token refresh, and storage in ESP32 Preferences
2. **CalendarHandler**: Queries Google Calendar API for bin collection events
3. **ConfigManager**: Persistent storage interface (WiFi credentials, calendar ID, device state) using ESP32 Preferences
4. **SetupServer**: Web server for initial device configuration (WiFi, OAuth, calendar selection)
5. **DisplayHandler**: LED matrix control (hardware interface)
6. **ButtonHandler**: Physical button with long-press detection

### Operating Modes

**Normal Mode**: Device connects to WiFi, polls calendar, displays bin status
**Setup Mode**: Device creates AP "Bindicator Setup", serves web UI at `bindicator.local` for configuration

Transitions into setup mode when:
- No WiFi credentials configured
- No OAuth refresh token available
- Forced setup flag set (via serial command)

### Calendar Event Detection

The system scans Google Calendar for events with titles containing:
- "Recycling" → Sets `RECYCLING_DUE`
- "Rubbish" → Sets `RUBBISH_DUE`

Events are checked for "today" to determine if bins need to go out.

### Persistent State

Device state persists across reboots via ESP32 Preferences:
- OAuth refresh token
- WiFi credentials
- Current device state
- Completion timestamp
- Bin taken out timestamp
- Configured calendar ID

### Serial Commands

The device accepts serial commands for debugging/control (see `serial_commands.cpp`). Commands trigger setup mode, factory reset, or status queries.

## Testing Strategy

Unit tests use GoogleTest with mocks for Arduino/FreeRTOS APIs. The test suite covers:
- State machine logic in `Bindicator`
- Time calculations in `time_manager`
- Configuration persistence in `ConfigManager`
- Utility functions

Tests compile production C++ code with `-DTESTING` flag and mock out hardware dependencies.

## Important Files

- `bindicator.ino`: Arduino sketch entry point (setup/loop)
- `bindicator.cpp/h`: Core state machine logic
- `secrets.h`: OAuth credentials and WiFi defaults (not committed)
- `tasks.cpp/h`: FreeRTOS task definitions and command queue
- `config_manager.cpp/h`: Persistent storage abstraction

## Code Structure Notes

- Header files use `#pragma once` (modern) or include guards (older files)
- All source files should start with `ABOUTME:` comments explaining purpose
- State changes must go through `Bindicator::transitionTo()` to ensure proper persistence and command queue updates
- OAuth tokens are automatically refreshed when expired (handled in `OAuthHandler`)
