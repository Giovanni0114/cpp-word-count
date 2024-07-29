CC = g++
CXXFLAGS = -std=c++20 -pthread 
TARGET = main
SOURCE = main.cpp

ifeq ($(DEBUG),1)
CXXFLAGS += -g -DDEBUG
endif

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

all_words:
	./create_all_words_file.sh

prepare: all_words
	python3 generate_file.py --file test_sets/test_01 --wordlist all_words --size 512 --mb 
	python3 generate_file.py --file test_sets/test_02 --wordlist all_words --size 1

	python3 generate_file.py --file test_sets/test_03 --random --size 1
	python3 generate_file.py --file test_sets/test_04 --random --size 1

	python3 generate_file.py --file test_sets/test_05 --random --wordcount 2137 --size 1
	python3 generate_file.py --file test_sets/test_06 --random --wordcount 100000 --size 2


test: $(TARGET)
	@for test in $(wildcard test_sets/test_0*); do \
		echo testing $$test; \
		./$(TARGET) $$test; \
	done

test_big_data: $(TARGET)
	./$(TARGET) test_sets/test_big

prepare_big_data: $(TARGET) all_words
	python3 generate_file.py --file test_sets/test_big --wordlist all_words --size 15

clean:
	rm -f $(TARGET)

.PHONY: all clean prepare prepare_big_data test test_big_data
