CXX = g++
CXXFLAGS = -std=c++14 -Wall -I./mocks -I.. $(shell pkg-config --cflags gtest)
GTEST_FLAGS = $(shell pkg-config --libs gtest gtest_main) -pthread

TEST_SRCS = unit/utils_test.cpp ../utils.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_BINS = unit/utils_test

.PHONY: all clean test

all: $(TEST_BINS)

$(TEST_BINS): $(TEST_OBJS)
	$(CXX) $(TEST_OBJS) -o $@ $(GTEST_FLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_BINS)
	./unit/utils_test

clean:
	rm -f $(TEST_OBJS) $(TEST_BINS) 