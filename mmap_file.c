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

#define NUM_CPUS 8

#define handle_error_en(en, msg) \
    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

/* Return a uniformly random number in the range [low,high]. */ 
inline int random_range (unsigned const low, unsigned const high) 
{
    unsigned const range = high - low + 1; 
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
    /* Stores user option for read/write/none */
    char touch;
    /* Stores user option for number of ops */
    int num_ops;
    /* Stores user option for file to mmap */
    char *file_path;
    size_t filesize;
    int fd; 
    char* file_memory; 
    int i;
    char a;
    int *random_series;
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
    if(argc != 4){
        printf("Usage: mmap-file </path/to/file> <r/w/n> <# operations>\n");
        printf("Second argument must be r(ead)/w(rite)/n(op)\n");
        exit(0);
    }
    else {
        touch = (char) argv[2][0];
        if (touch != 'r' && touch != 'w' && touch != 'n') {
            printf("Second argument must be r(ead)/w(rite)/n(op)\n");
            exit(0);
        }
        file_path = argv[1];
        num_ops = atoi(argv[3]);
        filesize = getFilesize(file_path);
    }

    printf("Mapped File: %s\n", file_path);
    printf("Size:%zu\n", filesize);
    printf("Per-page operation: %c\n", touch);
    printf("Number of operations: %d\n", num_ops);

    /* Create a list of random numbers from [0, filesize) */
    random_series = (int *) malloc(num_ops * sizeof(int));
    for(i=0; i<num_ops; i++) 
        random_series[i]= random_range(0, filesize-1);

    /* Prepare a file large enough to hold an unsigned integer. */ 
    fd = open (file_path, O_RDWR, S_IRUSR | S_IWUSR); 
    if (fd == -1) {
        perror("File cannot be opened!");
    }

    /*
    lseek (fd, random_range(0,filesize-1), SEEK_SET); 
    write (fd, "", 1); 
    lseek (fd, 0, SEEK_SET); 
    */
    /* Create the memory mapping. */ 
	clock_gettime( CLOCK_REALTIME, &start); 
    file_memory = mmap (0, filesize, PROT_WRITE, MAP_SHARED, fd, 0); 
	clock_gettime( CLOCK_REALTIME, &stop); 
    close (fd); 
	
    accum = ( stop.tv_sec - start.tv_sec )*1000000000  +
		(stop.tv_nsec - start.tv_nsec);
	printf( "mmap: %lf us\n", accum/1000 );

    if(touch != 'n') {
        for(i=0; i<num_ops; i++) {
            if(touch == 'w') 
                /* Write 'b' to a random addr in memory-mapped area. */ 
                file_memory[random_series[i]] =  'b'; 
            else
                /* Read from a random addr in memory-mapped area. */ 
                a = file_memory[random_series[i]];
        }
        printf("%d ops complete\n", num_ops);
    }

    //[4] print PID and buffer addresses:
    printf("PID:%d\n", pid);

    /* only here to avoid unused variable warning/error */
    if(touch == 'r')
        printf("a:%c\n", a);

    /* Release the memory (unnecessary because the program exits). */ 
    munmap (file_memory, filesize); 

    return 0; 
}  
