#!/bin/sh

ompThreads="1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16"

cmake .
make

for threads in $ompThreads
do
    echo "running with $threads threads"
    cmd="likwid-perfctr -C N:0-$(( $threads - 1 )) -g FLOPS_DP -f -m bin/code.app"
	echo $cmd
	eval $cmd
done
