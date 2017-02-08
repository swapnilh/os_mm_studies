#include <stdio.h>
#include <sys/prctl.h>
#include <time.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

int temp_data = 100;
static int temp_bss;

void print_addr ( void )
{
        int local_var = 100;
        int *code_segment_address = ( int* ) &print_addr;
        int *data_segment_address = &temp_data;
        int *bss_address = &temp_bss;
        int *stack_segment_address = &local_var;

        printf ( "\nAddress of various segments:" );
        printf ( "\n\tCode Segment : %p" , code_segment_address );
        printf ( "\n\tData Segment : %p" , data_segment_address );
        printf ( "\n\tBSS : %p" , bss_address );
        printf ( "\n\tStack Segment : %p\n" , stack_segment_address );

}

int main ( )
{
        print_addr ();
        uintptr_t addr = (uintptr_t) mmap(NULL,4194304, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
        printf("mmap-ed region:%" PRIx64 "\n", addr); 
        int status = prctl(PR_SET_MM, PR_SET_MM_START_CODE, addr, 0, 0);
        if (status<0)
            perror("prctl failed!");
        print_addr ();
        return 0;
}
