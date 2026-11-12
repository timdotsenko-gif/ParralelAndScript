#!/bin/bash

echo "threads,time_sec"
for T in 1 2 4 6 8 12; do
  time_sec=$(./rec100k $T | grep -oP 'Time: \K[0-9.]+')
  echo "$T,$time_sec"
done
