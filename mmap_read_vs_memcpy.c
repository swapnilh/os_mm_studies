// Source based on http://www.makelinux.net/alp/037
#define _GNU_SOURCE //Gives access to CPU_ZERO, CPU_SET
#include <stdlib.h> 
#include <stdio.h> 
#include <fcntl.h> 
#include <sys/mman.h> 
#include <sys/stat.h> 
#include <time.h> 
#include <unistd.h> 
#include <errno.h> 
#include <pthread.h> 
#include <string.h> 
#include <assert.h> 

#define NUM_CPUS 8
#define CHUNK_SIZE 16384 

#define handle_error_en(en, msg) \
    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

/* Return a uniformly random number in the range [low,high]. */ 
inline int random_range (unsigned long low, unsigned long high) 
{
    unsigned long range = high - low + 1; 
    return low + (int) (((double) range) * rand () / (RAND_MAX + 1.0)); 
} 

size_t getFilesize(const char* filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

int main (int argc, char* const argv[]) 
{
    // pid: the process ID of this process 
    // so we can print it out
    int pid;
    // for clock_gettime
	struct timespec start, stop;
	double accum;
    cpu_set_t cpuset;
    pthread_t thread;
    int s, core; 
    char touch;         /* Stores user option for read/memcpy */
    char *file_path;    /* Stores user option for file to mmap */
    size_t filesize;
    int fd; 
    char* file_memory; 
    ssize_t read_in;    /* Number of bytes read in */
    char buffer[CHUNK_SIZE]; /* buffer to store the read bytes in */
    int iter = 0; /* Number of iterations for reading complete file */

    /* Seed the random number generator. */ 
    srand (time (NULL)); 

    /* Pin process to a random CPU */
    core=random_range(0,NUM_CPUS-1);
    thread = pthread_self();
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);
    s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (s != 0)
        handle_error_en(s, "pthread_setaffinity_np");
    
    printf("Pinned to CPU%d\n", core);

    pid = getpid();
    if(argc != 3){
        printf("Usage: mmap-file </path/to/file> <m/r>\n");
        printf("Second argument must be r(ead syscall)/m(emcpy)\n");
        exit(0);
    }
    else {
        touch = (char) argv[2][0];
        if (touch != 'r' && touch != 'm') {
            printf("Second argument must be r(ead syscall)/m(emcpy)\n");
            exit(0);
        }
        file_path = argv[1];
        filesize = getFilesize(file_path);
    }

    printf("Mapped File: %s\n", file_path);
    printf("Size:%zu\n", filesize);
    printf("Per-page operation: %c\n", touch);

    fd = open (file_path, O_RDWR, S_IRUSR | S_IWUSR); 
    if (fd == -1) {
        perror("File cannot be opened!");
    }

    /* Create the memory mapping. */ 
    if(touch == 'm') {
        clock_gettime( CLOCK_REALTIME, &start); 
        file_memory = mmap (0, filesize, PROT_READ, MAP_SHARED, fd, 0); 
        clock_gettime( CLOCK_REALTIME, &stop); 
    }

    accum = ( stop.tv_sec - start.tv_sec )*1000000000  +
		(stop.tv_nsec - start.tv_nsec);
	printf( "mmap: %lf us\n", accum/1000 );

    /* Read file in CHUNK_SIZE chunks using read syscall */
    if(touch == 'r') {
        clock_gettime( CLOCK_REALTIME, &start); 
        while(filesize> 0) {
            read_in = read(fd, &buffer, CHUNK_SIZE);
            assert(read_in>0);
            filesize-=CHUNK_SIZE;
            iter++;
        }
        clock_gettime( CLOCK_REALTIME, &stop); 
    }
    /* Read file in CHUNK_SIZE chunks using memcpy */
    else {
        clock_gettime( CLOCK_REALTIME, &start); 
        while(filesize> 0) {
            memcpy(&buffer, file_memory, CHUNK_SIZE);
            file_memory+=CHUNK_SIZE;
            filesize-=CHUNK_SIZE;
            iter++;
        }
        clock_gettime( CLOCK_REALTIME, &stop); 
    }

    accum = ( stop.tv_sec - start.tv_sec )*1000000000  +
		(stop.tv_nsec - start.tv_nsec);
	printf( "Read/memcpy time: %lf us\n", accum/1000 );

    //[4] print PID and buffer addresses:
    printf("PID:%d\n", pid);
    printf("Iterations taken:%d\n", iter);
    printf("Sanity Check : Buffer output\n");

    int i;
    for(i=0; i<10; i++)
        printf("%c ",buffer[i]);
    printf("\n");
    /* Release the memory (unnecessary because the program exits). */ 
    if(touch == 'm')
        munmap (file_memory, filesize); 

    close (fd); 

    return 0; 
}  
