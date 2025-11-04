CXX = g++
CXXFLAGS = -std=c++14 -Wall -Werror -DSIMULATOR
CXXFLAGS += -Imocks -Imocks/freertos -I..
LDFLAGS = -pthread -lcurl

TARGET = simulator
SRCS = main.cpp simulated_time.cpp terminal_display.cpp
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
