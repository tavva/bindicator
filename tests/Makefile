CXX = g++
CXXFLAGS = -std=c++14 -Wall -I./mocks -I./mocks/freertos -I.. -DTESTING $(shell pkg-config --cflags gtest)
GTEST_FLAGS = $(shell pkg-config --libs gtest gtest_main) -pthread

TEST_SRCS = unit/utils_test.cpp \
            unit/bindicator_test.cpp \
            unit/time_manager_test.cpp \
            unit/config_manager_test.cpp \
            mocks/freertos_mock.cpp \
            mocks/Arduino.cpp \
            mocks/time_mock.cpp \
            ../utils.cpp \
            ../bindicator.cpp \
            ../time_manager.cpp \
            ../config_manager.cpp

TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_BINS = unit/test_runner

.PHONY: all clean test

all: $(TEST_BINS)

$(TEST_BINS): $(TEST_OBJS)
	$(CXX) $(TEST_OBJS) -o $@ $(GTEST_FLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_BINS)
	./unit/test_runner

clean:
	rm -f $(TEST_OBJS) $(TEST_BINS) 