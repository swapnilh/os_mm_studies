SRCS = mmap mmap_ftrace mmap_file mmap_read_vs_memcpy identity_test segment_map mov_code_segment identity_test_file pthread_ex
LIBS = -lpthread
all: ${SRCS}

CC = gcc
CFLAGS = -O2 -Wall -g
PIE_CFLAGS = -fpie -pie

segment_map: segment_map.c
	${CC} ${CFLAGS} ${PIE_CFLAGS} -o $@ $< ${LIBS} 

mov_code_segment: mov_code_segment.c
	${CC} ${CFLAGS} ${PIE_CFLAGS} -o $@ $< ${LIBS} 

%:%.c
	${CC} ${CFLAGS} -o $@ $<  ${LIBS} 

clean:
	rm -f ${SRCS} *.o *.s 
