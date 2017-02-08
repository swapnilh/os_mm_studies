num_operations=$1
sizes=( "400K" "4M" "400M" "4G" "40G" )
for i in "${sizes[@]}"
do
    echo 1 | sudo tee /proc/sys/vm/drop_caches
    #Understand the number of TLB misses/page fault in the baseline (with no accesses to the mmapped region)
    perf stat -e dTLB-load-misses,iTLB-load-misses ./mmap_file /nobackup/swapnilh/random_files/file_${i}.dat n ${num_operations}
    echo 1 | sudo tee /proc/sys/vm/drop_caches
    perf stat -e dTLB-load-misses,iTLB-load-misses ./mmap_file /nobackup/swapnilh/random_files/file_${i}.dat w ${num_operations}
done
