#!/bin/bash
for i in `seq 0 5`
do
    # nop clears the buffer
    echo nop | sudo tee /sys/kernel/debug/tracing/current_tracer
    echo function_graph | sudo tee /sys/kernel/debug/tracing/current_tracer
    sudo ./mmap_ftrace $i r
    sudo cat /sys/kernel/debug/tracing/trace | tee trace_THP_${i}.log
    cat trace_THP_${i}.log | awk '/Before mprotect/ { show=1 } show; /After mprotect/ { show=0 }' > trace_THP_${i}_app.log
    mv trace_THP_${i}_app.log trace_THP_${i}.log
done
