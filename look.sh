#!/bin/bash

step=2 #间隔的秒数，不能大于60

for (( i = 0; i > -10; i=(i+step) )); do
    echo ------------$(date)------------ 
    netstat -n |grep 2222 |awk '/^tcp/ {++S[$NF]} END {for(a in S) print a, S[a]}'
    sleep $step
done

exit 0
