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
