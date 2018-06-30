#!/bin/sh

ompThreads="1 2 3 4 5 6 7 8"

cmake .
make

for threads in $ompThreads
do
    echo "running with $threads omp threads"
    export OMP_NUM_THREADS=$threads
    bin/code.app
done
