CC = g++
CXXFLAGS = -pthread
TARGET = main
SOURCE = main.cpp

ifeq ($(DEBUG),1)
CXXFLAGS += -g
endif

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

prepare: all_words
	python3 generate_file.py --file test_sets/test_01 --wordlist all_words --size 512 --mb 
	python3 generate_file.py --file test_sets/test_02 --wordlist all_words --size 1

	python3 generate_file.py --file test_sets/test_03 --random --size 1
	python3 generate_file.py --file test_sets/test_04 --random --size 1

	python3 generate_file.py --file test_sets/test_05 --random --wordcount 2137 --size 1
	python3 generate_file.py --file test_sets/test_06 --random --wordcount 100000 --size 2

all_words:
	./create_all_words_file.sh

test: $(TARGET)
	./$(TARGET) test_sets/test_01
	./$(TARGET) test_sets/test_02
	./$(TARGET) test_sets/test_03
	./$(TARGET) test_sets/test_04
	./$(TARGET) test_sets/test_05
	./$(TARGET) test_sets/test_06

test_big_data: $(TARGET)
	./$(TARGET) test_sets/test_11
	./$(TARGET) test_sets/test_12
	./$(TARGET) test_sets/test_13

prepare_big_data: $(TARGET) all_words
	python3 generate_file.py --file test_sets/test_11 --wordlist all_words --size 15
	python3 generate_file.py --file test_sets/test_12 --wordlist all_words --size 10
	python3 generate_file.py --file test_sets/test_12 --wordlist all_words --size 20

clean:
	rm -f $(TARGET) $(TEST_FILE)

.PHONY: all clean prepare prepare_big_data test test_big_data
