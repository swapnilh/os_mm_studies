SRCS = mmap mmap_ftrace mmap_file mmap_read_vs_memcpy mmap_see_vm identity_test segment_map mov_code_segment
LIBS = -lpthread
all: ${SRCS}

CC = gcc
CFLAGS = -O2 -Wall -g
PIE_CFLAGS = -fpic -pie

segment_map: segment_map.c
	${CC} ${CFLAGS} ${PIE_CFLAGS} -o $@ $< ${LIBS} 

mov_code_segment: mov_code_segment.c
	${CC} ${CFLAGS} ${PIE_CFLAGS} -o $@ $< ${LIBS} 

%:%.c
	${CC} ${CFLAGS} -o $@ $<  ${LIBS} 

clean:
	rm -f ${SRCS} *.o *.s 
