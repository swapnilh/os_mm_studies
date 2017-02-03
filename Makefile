SRCS = mmap mmap_ftrace mmap_file mmap_read_vs_memcpy mmap_see_vm identity_mapping
LIBS = -lpthread
all: ${SRCS}

CC = gcc
CFLAGS = -O0 -Wall -g

%:%.c
	${CC} ${CFLAGS} -o $@ $<  ${LIBS} 

clean:
	rm -f ${SRCS} *.o *.s 
