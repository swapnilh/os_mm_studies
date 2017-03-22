#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS	5
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


void *PrintHello(void *threadid)
{
   long tid;
   tid = (long)threadid;
   printf("Hello World! It's me, thread #%ld!\n", tid);
   printf("Stack VA:%p PA:%" PRIx64 "\n", &tid, virtual_to_physical_address((uintptr_t) &tid));
   pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
   pthread_t threads[NUM_THREADS];
   int rc;
   long t;
   for(t=0;t<NUM_THREADS;t++){
     printf("In main: creating thread %ld\n", t);
     rc = pthread_create(&threads[t], NULL, PrintHello, (void *)t);
     if (rc){
       printf("ERROR; return code from pthread_create() is %d\n", rc);
       exit(-1);
       }
     }
	
   while(1);
   /* Last thing that main() should do */
   pthread_exit(NULL);
}
