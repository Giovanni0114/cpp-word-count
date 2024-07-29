CC = g++
CXXFLAGS = -pthread
TARGET = main
SOURCE = main.cpp
TEST_FILE = large_test_file.txt
TEST_SIZE_GB = 1

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

generate_test_file:
	python3 generate_file.py $(TEST_FILE) $(TEST_SIZE_GB)

test: $(TARGET) generate_test_file
	./$(TARGET) $(TEST_FILE)

clean:
	rm -f $(TARGET) $(TEST_FILE)

.PHONY: all generate_test_file test clean
