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
            mocks/ArduinoJson.cpp mocks/secrets.cpp \
            mocks/Adafruit_NeoPixel.cpp \
            mocks/freertos/queue.cpp mocks/freertos/task.cpp

# Firmware sources (relative to parent directory)
FIRMWARE_SRCS = ../bindicator.cpp ../config_manager.cpp \
                ../oauth_handler.cpp ../calendar_handler.cpp \
                ../tasks.cpp ../time_manager.cpp \
                ../utils.cpp ../button_handler.cpp \
                ../serial_commands.cpp ../setup_server.cpp \
                ../setup_server_page_generation.cpp \
                ../display_handler.cpp ../animations.cpp

SRCS = $(SIM_SRCS) $(MOCK_SRCS) $(FIRMWARE_SRCS)
OBJS = $(SIM_SRCS:.cpp=.o) $(MOCK_SRCS:.cpp=.o)
FIRMWARE_OBJS = $(notdir $(FIRMWARE_SRCS:.cpp=.o))

# Add the .ino file separately (compiled as bindicator_main.o to avoid conflict)
FIRMWARE_OBJS += bindicator_main.o

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS) $(FIRMWARE_OBJS)
	$(CXX) $(OBJS) $(FIRMWARE_OBJS) -o $(TARGET) $(LDFLAGS)

# Simulator and mock objects
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Firmware objects (compile from parent directory)
# .ino file (Arduino treats as .cpp) - renamed to avoid conflict with bindicator.cpp
bindicator_main.o: ../bindicator.ino
	$(CXX) $(CXXFLAGS) -x c++ -c $< -o $@

bindicator.o: ../bindicator.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

config_manager.o: ../config_manager.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

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

setup_server_page_generation.o: ../setup_server_page_generation.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

display_handler.o: ../display_handler.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

animations.o: ../animations.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(FIRMWARE_OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)
