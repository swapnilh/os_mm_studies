SRCS = mmap mmap_ftrace mmap_file 
LIBS = -lpthread
all: ${SRCS}

CC = gcc
CFLAGS = -O0 -Wall -g

%:%.c
	${CC} ${CFLAGS} -o $@ $<  ${LIBS} 

clean:
	rm -f ${SRCS} *.o *.s 
