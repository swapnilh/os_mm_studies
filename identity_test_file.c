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
#include <inttypes.h>
#include <emmintrin.h>
#include <fcntl.h>

#define PAGEMAP_ENTRY 8
#define GET_BIT(X,Y) (X & ((uint64_t)1<<Y)) >> Y
#define GET_PFN(X) X & 0x7FFFFFFFFFFFFF
#define page_mapping_file "/proc/self/pagemap"
#define NUM_CPUS 8
#define handle_error_en(en, msg) \
    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

inline int random_range (unsigned const low, unsigned const high) 
{
    unsigned const range = high - low + 1; 
    return low + (int) (((double) range) * rand () / (RAND_MAX + 1.0)); 
}
const int __endian_bit = 1;
#define is_bigendian() ( (*(char*)&__endian_bit) == 0 )

uintptr_t virtual_to_physical_address(uintptr_t virt_addr)
{
   uintptr_t file_offset = 0;
   uintptr_t read_val = 0;
   int i = 0;
   int c = 0;
   int status = 0;
   unsigned char c_buf[PAGEMAP_ENTRY];

   FILE *f = fopen(page_mapping_file, "rb");
   if(!f)
   {
      // if this happens run as root
      printf("Error! Cannot open %s. Please, run as root.\n", page_mapping_file);
      return 0;
   }

   file_offset = virt_addr / getpagesize() * PAGEMAP_ENTRY;

   status = fseek(f, file_offset, SEEK_SET);
   if(status)
   {
      printf("Error! Cannot seek in %s.\n", page_mapping_file);
      perror("Failed to do fseek!");
      fclose(f);
      return 0;
   }

   for(i = 0; i < PAGEMAP_ENTRY; i++)
   {
      c = getc(f);
      if(c == EOF)
      {
         fclose(f);
         return 0;
      }

      if(is_bigendian())
      {
           c_buf[i] = c;
      }
      else
      {
           c_buf[PAGEMAP_ENTRY - i - 1] = c;
      }
   }

   for(i=0; i < PAGEMAP_ENTRY; i++)
   {
      read_val = (read_val << 8) + c_buf[i];
   }
   
   //printf("Raw:%" PRIx64 "\n", read_val);
   if (read_val & (1ULL << 63)) { // page present ?
       read_val = read_val & ((1ULL << 55) - 1); // pfn mask
       read_val = read_val * sysconf(_SC_PAGESIZE);
       // add offset within page
       read_val = read_val | (virt_addr & (sysconf(_SC_PAGESIZE) - 1));
   }   
   else {
       printf("Page not present\n");
   }

   fclose(f);

   return read_val;
}

int main( int argc, char *argv[]){
    // pid: the process ID of this process 
    // so we can print it out
    cpu_set_t cpuset;
    pthread_t thread;
    int s, core; 
    /* 4KB, 512KB, 4MB, 128MB, 512MB, 1GB, 4GB */
    size_t size_sel;
    size_t sizes[] = {4096, 524288, 4194304, 134217728, 536870912, 1073741824, 4294967296};
    uintptr_t i;

    /* Pin process to a random CPU */
    srand(time(NULL));
    core=random_range(0, NUM_CPUS-1);
    thread = pthread_self();
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);
    s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (s != 0)
        handle_error_en(s, "pthread_setaffinity_np");
    
    printf("Pinned to CPU%d\n", core);

    if(argc != 2){
        printf("Usage: mmap <size:1-5> \n");
        printf("0. 4KB\n1. 400KB\n2. 4MB\n3. 400MB\n4. 4GB\n5. 40GB\n");
        exit(0);
    }
    else if (atoi(argv[1])<=5){
        size_sel = sizes[atoi(argv[1])];
    }
    else if (atoi(argv[1])>5) {
	size_sel = atoi(argv[1]);
    }	

    printf("Size: %zu\n", size_sel);

    //[1] create a pointer in order to allocate 
    //memory region
    char *buffer;
    int fd; 
    
    fd = open ("/home/swapnil/os_mm_studies/file_4G.dat", O_RDWR, S_IRUSR | S_IWUSR); 
    if (fd == -1) {
        perror("File cannot be opened!");
    }
  
    //protected buffer:
    //allocate memory with mmap()
    buffer = (char *) mmap(0,
                      size_sel,
                      PROT_READ|PROT_WRITE,
//                      MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE|MAP_POPULATE,
                      MAP_PRIVATE|MAP_POPULATE,
                      fd, 0);
    if (buffer == MAP_FAILED) {
	perror("MMAP failed!");
	exit(1);
    }	
    uintptr_t phys_addr = virtual_to_physical_address((uintptr_t)buffer);
    printf("Before remap vaddr:%p, paddr:%" PRIx64 "\n", buffer, phys_addr);
    printf("Before remap vaddr:%p, paddr:%" PRIx64 "\n", buffer+size_sel-4096, virtual_to_physical_address((uintptr_t)buffer+size_sel-4096));
    buffer = (char *) mremap(buffer, size_sel, size_sel, MREMAP_FIXED|MREMAP_MAYMOVE, phys_addr);
    if (buffer == MAP_FAILED) 
        perror("remap failed");
    for(i=(uintptr_t)buffer; i<=(uintptr_t) buffer+size_sel-4096; i+=4096) {
/*	phys_addr = virtual_to_physical_address((uintptr_t)i);
	if(i != phys_addr) {
	    printf("Error! Couldn't map to physical address.\n");
	    printf("%d (-1) pages mapped\n", (int)((char *)i-buffer)/4096);
	    break;
	}
        //printf("After remap vaddr:%" PRIx64 ", paddr:%" PRIx64 "\n", i, phys_addr);
        //
        */
    }
    phys_addr = virtual_to_physical_address((uintptr_t)buffer);
    printf("After remap vaddr:%p, paddr:%" PRIx64 "\n", buffer, phys_addr);
    printf("After remap vaddr:%p, paddr:%" PRIx64 "\n", buffer+size_sel-4096, virtual_to_physical_address((uintptr_t)buffer+size_sel-4096));
    if ((uintptr_t) buffer != phys_addr)
	    printf("Error! Couldn't map to physical address.\n");
    phys_addr = virtual_to_physical_address((uintptr_t)buffer+size_sel-4096);
    if ((uintptr_t) buffer+size_sel-4096 != phys_addr)
        printf("Error! Couldn't map to physical address.\n");
    while(1);	
    munmap(buffer, size_sel);
    close (fd); 
    return 0;
}
