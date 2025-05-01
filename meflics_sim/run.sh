#!/bin/bash

# 실행 명령어
# Cache policy : FIFO, LRU, LFU, ARC, TwoQ, LHD, LeCaR
# ./build/MEFLICS_SIM workload_file cache_size first_policy second_policy

echo "Running..."

# 캐시 비우기 함수
clear_cache() {
    echo "Clearing cache..."
    sync; echo 3 > /proc/sys/vm/drop_caches
}

# 정책 실행 함수
run_policy() {
    local first_policy=$1
    local second_policy=$2
    local result_file=$3

    echo "Starting policy: $first_policy-$second_policy"
    clear_cache

    start_time=$(date +%s)
    echo "Start time: $(date)"

    ./build/MEFLICS_SIM /home/choi_gunhee/data/merged_kvcache_2206.csv 8 "$first_policy" "$second_policy" >> "$result_file"

    end_time=$(date +%s)
    echo "End time: $(date)"

    elapsed_time=$((end_time - start_time))
    echo "Elapsed time: ${elapsed_time}s"
    echo "Finished policy: $first_policy-$second_policy"
    echo "----------------------------------------"
}

# 실행
run_policy FIFO FIFO meflics_8GB_FIFO_FIFO.result
run_policy FIFO LRU meflics_8GB_FIFO_LRU.result
run_policy FIFO LFU meflics_8GB_FIFO_LFU.result
run_policy FIFO ARC meflics_8GB_FIFO_ARC.result
run_policy FIFO TwoQ meflics_8GB_FIFO_TwoQ.result
run_policy FIFO LHD meflics_8GB_FIFO_LHD.result
run_policy FIFO LeCaR meflics_8GB_FIFO_LeCaR.result