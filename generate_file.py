#!/usr/bin/python3

import argparse
import string
import random
import sys

path = "/usr/share/dict/words"

def generate_random_word(min_length=2, max_length=16):
    return ''.join(random.choices(string.ascii_lowercase, k=random.randint(min_length, max_length)))

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate a large text file with random words.')
    parser.add_argument('-f', '--file', type=argparse.FileType('w'), help='The file where the sum should be written', default=sys.stdout)
    parser.add_argument('-s', '--size', type=float, help='The size of the file.', default=1)
    parser.add_argument('-w', '--wordlist', type=argparse.FileType('r'), help='File with all words to be used in file', default="/usr/share/dict/words")

    parser.add_argument('-r', '--random', action='store_true', help='Use random words')
    parser.add_argument('-m', '--mb', action='store_true', help='Size in megabytes.')

    args = parser.parse_args()

    size_in_bytes = args.size * 1024 ** 2

    if not args.mb:
        size_in_bytes *= 1024

    try:
        words = args.wordlist.read().splitlines()
    except FileNotFoundError:
        print(f"Word file '{path}' not found.")
        sys.exit(1)

    bytes_written = 0
    while bytes_written < size_in_bytes:
        line = ' '.join(random.choices(words, k=100)) + " "
        args.file.write(line)
        bytes_written += len(line.encode('utf-8'))

