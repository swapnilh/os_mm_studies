#!/bin/bash
if [ $# != 1 ]; then
    echo "1 argument required"
    echo "Usage: $0 r/m"
    echo "r: read syscall, m:memcpy"
    exit
fi
op=$1
echo "op:", $op
sizes=( "400K" "4M" "400M" "4G" "40G" )
for j in `seq 1 5` ;
do
    # only clear pagecache when reading the same file again i.e in new iteration
    echo 1 | sudo tee /proc/sys/vm/drop_caches
    for i in "${sizes[@]}";
    do
        #Understand the number of TLB misses/page fault in the baseline (with no accesses to the mmapped region)
#        perf stat -e dTLB-load-misses,dTLB-store-misses ./mmap_read_vs_memcpy /nobackup/swapnilh/random_files/file_${i}.dat $1
        ./mmap_read_vs_memcpy /nobackup/swapnilh/random_files/file_${i}.dat $1
    done
done 
