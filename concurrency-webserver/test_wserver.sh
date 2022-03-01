#! /usr/bin/env bash

IP_ADDR=192.168.0.140
PORT=10000
N=10;

# retrieve large file
for ((i=1;i<=$N;i++))
do
    curl $IP_ADDR:$PORT/shakespeare.txt & 
done


for ((i=1;i<=$N;i++))
do
    curl $IP_ADDR:$PORT/adder.cgi?$i\&$i & 
done
