#!/bin/bash

echo "Running..."

WORKLOAD="/home/choi_gunhee/data/merged_kvcache_2206.csv"
EXEC="./build/BASIC_SIM"

clear_cache() {
    echo "Clearing Linux cache..."
    sudo sync
    sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
    echo "Cache cleared."
}

# FIFO
echo "FIFO..."
clear_cache
$EXEC $WORKLOAD 8 0 >> basic_8GB_fifo.result

# LRU
echo "LRU..."
clear_cache
$EXEC $WORKLOAD 8 1 >> basic_8GB_lru.result

# LFU
echo "LFU..."
clear_cache
$EXEC $WORKLOAD 8 2 >> basic_8GB_lfu.result

# ARC
echo "ARC..."
clear_cache
$EXEC $WORKLOAD 8 3 >> basic_8GB_arc.result

# Two-Q
echo "Two-Q..."
clear_cache
$EXEC $WORKLOAD 8 4 >> basic_8GB_twoq.result

# LHD
echo "LHD..."
clear_cache
$EXEC $WORKLOAD 8 5 >> basic_8GB_LHD.result

# LeCaR
echo "LeCaR..."
clear_cache
$EXEC $WORKLOAD 8 6 >> basic_8GB_LeCaR.result


echo "Done."
