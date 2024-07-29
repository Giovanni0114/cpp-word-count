#!/usr/bin/python3

import sys

def count_distinct_words(file_path):
    distinct_words = set()

    with open(file_path, 'r') as file:
        for line in file:
            words = line.split()
            for word in words:
                distinct_words.add(word)

    return len(distinct_words)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <filename> ")
        sys.exit(1)

    print(f'The number of distinct words in the file is: {count_distinct_words(sys.argv[1])}')
