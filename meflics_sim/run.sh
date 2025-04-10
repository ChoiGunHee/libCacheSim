#!/bin/bash

echo "Running..."

# 실행 명령어

# echo "FIFO..."
# echo "---------------------------------------------------------------"
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 1 0
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 2 0
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 4 0
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 8 0
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 16 0

# echo "LRU..."
# echo "---------------------------------------------------------------"
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 1 1
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 2 1
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 4 1
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 8 1
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 16 1

# echo "LFU..."
# echo "---------------------------------------------------------------"
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 1 2
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 2 2
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 4 2
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 8 2
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 16 2

# echo "ARC..."
# echo "---------------------------------------------------------------"
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 1 3
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 2 3
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 4 3
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 8 3
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 16 3

# echo "TwoQ..."
# echo "---------------------------------------------------------------"
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 1 4
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 2 4
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 4 4
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 8 4
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 16 4

# echo "S3-FIFO..."
# echo "---------------------------------------------------------------"
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 1 5
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 2 5
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 4 5
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 8 5
# ./gli_cache /home/choi_gunhee/data/merged_kvcache_2206.csv 16 5

./build/MEFLICS_SIM /home/choi_gunhee/my_project/libCacheSim/data/test.csv  1 7