#define _GNU_SOURCE //Gives access to CPU_ZERO, CPU_SET
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>

#define NUM_CPUS 8
#define handle_error_en(en, msg) \
    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

inline int random_range (unsigned const low, unsigned const high) 
{
    unsigned const range = high - low + 1; 
    return low + (int) (((double) range) * rand () / (RAND_MAX + 1.0)); 
} 

int main( int argc, char *argv[]){
    // pid: the process ID of this process 
    // so we can print it out
    int pid;
    // for clock_gettime
	struct timespec start, stop;
    //size: an integer to hold the current region size
    size_t size_sel;
	double accum;
    cpu_set_t cpuset;
    pthread_t thread;
    int s, core; 
    /* Stores user option for read/write/none */
    char touch;
    char a;
    /* 4KB, 400KB, 4MB, 400MB, 4GB, 40GB */
    size_t sizes[] = {4096, 409600, 4194304, 419430400, 4294967296, 42949672960};

    /* Pin process to a random CPU */
    srand(time(NULL));
    core=random_range(0, NUM_CPUS);
    thread = pthread_self();
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);
    s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (s != 0)
        handle_error_en(s, "pthread_setaffinity_np");
    
    printf("Pinned to CPU%d\n", core);
    /*
    s = pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (s != 0)
        handle_error_en(s, "pthread_getaffinity_np");
    */

    pid = getpid();
    if(argc != 3){
        printf("Usage: mmap <size:1-5> <r/w/n>\n");
        printf("0. 4KB\n1. 400KB\n2. 4MB\n3. 400MB\n4. 4GB\n5. 40GB\n");
        printf("Second argument must be r(ead)/w(rite)/n(op)\n");
        exit(0);
    }
    else {
        size_sel = sizes[atoi(argv[1])];
        touch = (char) argv[2][0];
        if (touch != 'r' && touch != 'w' && touch != 'n') {
            printf("Second argument must be r(ead)/w(rite)/n(op)\n");
            exit(0);
        }
    }

    printf("Size: %zu\n", size_sel);
    printf("Per-page operation: %c\n", touch);

    //[1] create a pointer in order to allocate 
    //memory region
    char *buffer;
  
    //protected buffer:
    //allocate memory with mmap()
	clock_gettime( CLOCK_REALTIME, &start); 
    buffer = (char *) mmap(NULL,
                      size_sel,
                      PROT_READ|PROT_WRITE,
                      MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE,
                      0,0);
	clock_gettime( CLOCK_REALTIME, &stop); 
	accum = ( stop.tv_sec - start.tv_sec )*1000000000  +
		(stop.tv_nsec - start.tv_nsec);
	printf( "mmap: %lf us\n", accum/1000);

    //[2] put some characters in the allocated memory

    if (touch != 'n') {
        char *itr = buffer;
        for(; itr<buffer+size_sel;itr+=4096)
            if(touch == 'w')
                *itr = 'a';
            else
                a=*itr;
    }
    
    //[3] protect the page with PROT_NONE:
	clock_gettime( CLOCK_REALTIME, &start); 
    mprotect(buffer, size_sel, PROT_NONE);
	clock_gettime( CLOCK_REALTIME, &stop); 
	accum = ( stop.tv_sec - start.tv_sec )*1000000000  +
		(stop.tv_nsec - start.tv_nsec);
	printf( "mprotect: %lf us\n", accum/1000 );

    //[4] print PID and buffer addresses:
    printf("buffer at %p\n", buffer);
    printf("PID:%d\n", pid);
    /* only here to avoid unused variable warning/error */
    printf("a:%c\n", a);
    
    //spin until killed so that we know it's in memory:
    while(1);

    //Making sure mmap/mprotect is not optimized out
    *buffer = 'a';

    return 0;
}
