#!/bin/bash

for file in test_sets/test_0*; do
    echo $file
    ./slow_count.py $file
done
