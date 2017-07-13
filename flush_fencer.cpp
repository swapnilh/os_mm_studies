#include<iostream>
#include<cassert>
#include<x86intrin.h>

// COMPILE WITH -O0 to ensure no flush/fences are optimized away

constexpr auto MEM_SIZE = 1024*1024*1024;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Error! Usage: flush_fencer <# flush> <# fence>\n";
        return 1;
    }

    int num_flushes = atoi(argv[1]);
    int num_fences = atoi(argv[2]);
    int dummy_sum = 0;    
    int *scratchpad = new int[MEM_SIZE/sizeof(int)];

    for (int i = 0; (num_flushes-- > 0 && i < MEM_SIZE/sizeof(int)); i+=64) {
        scratchpad[i] = i;
        _mm_clwb((void*)&(scratchpad[i]));
    }

    for (int i = 0; (num_fences-- >0 && i < MEM_SIZE/sizeof(int)); i+=64) {
        scratchpad[i] = 2*i+1;
        _mm_lfence();
	dummy_sum += scratchpad[i];
    }

    assert(dummy_sum!=0);

    return 0;
}
