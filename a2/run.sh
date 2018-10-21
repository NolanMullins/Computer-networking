#!/bin/bash
# ip:port file <number of time to run>
if (($# < 3)); 
then
    echo "<IP:Port> <file> <# of runs> <transfer size>"
    exit -1
fi
for ((i=0; i<$3; i++))
do
    ./client $1 $2 $4
done
