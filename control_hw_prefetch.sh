#!/bin/bash
# Should work on Intel processors based on these microarchitectures:
# Nehalem, Westmere, Sandy Bridge, Ivy Bridge, Haswell, and Broadwell.
# Based on information see here-
# https://software.intel.com/en-us/articles/disclosure-of-hw-prefetcher-control-on-some-intel-processors
if [ $# != 1 ]; then
    echo "1 argument required"
    echo "Usage: $0 d/e/s"
    echo "d:disable h/w prefetch, e:enable h/w prefetch; s:status"
    exit
fi
if [ "$1" != "d" ] && [ "$1" != "e" ] && [ "$1" != "s" ]; then
    echo "Only d/e/s accepted"
    echo "Usage: $0 d/e/s"
    echo "d:disable h/w prefetch, e:enable h/w prefetch; s:status"
    exit
fi
if [ "$1" = "d" ]; then
    echo "Disabling h/w prefetchers!"
    for i in `seq 0 7`
    do
        sudo wrmsr -p $i 0x1a4 0xf
    done
fi
if [ "$1" = "e" ]; then
    echo "Enabling h/w prefetchers!"
    for i in `seq 0 7`
    do
        sudo wrmsr -p $i 0x1a4 0x0
    done
fi
if [ "$1" = "s" ]; then
    echo "Current status of h/w prefetchers!"
    for i in `seq 0 7`
    do
        sudo rdmsr -p $i 0x1a4
    done
fi
