CXX = g++
CXXFLAGS = -std=c++14 -Wall -Werror -DSIMULATOR
CXXFLAGS += -Imocks -Imocks/freertos -I..
LDFLAGS = -pthread -lcurl

TARGET = simulator
SRCS = main.cpp simulated_time.cpp terminal_display.cpp \
       mocks/Arduino.cpp mocks/Preferences.cpp \
       mocks/WiFi.cpp mocks/HTTPClient.cpp \
       mocks/ESPmDNS.cpp mocks/ESP.cpp mocks/DisplayHandler.cpp \
       mocks/freertos/queue.cpp mocks/freertos/task.cpp
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
