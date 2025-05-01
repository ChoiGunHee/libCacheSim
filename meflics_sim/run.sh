#!/bin/bash

# 실행 명령어
#Cache policy : FIFO, LRU, LFU, ARC, TwoQ, LHD, LeCaR
# ./build/MEFLICS_SIM workload_file cache_size first_policy second_policy

echo "Running..."

./build/MEFLICS_SIM /home/choi_gunhee/my_projects/libCacheSim/data/test.csv 8 LHD LRU
