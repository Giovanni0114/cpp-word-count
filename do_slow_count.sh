#!/bin/bash

for file in test_sets/test_0*; do
    ./slow_count.py $file
done
