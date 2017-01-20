SRCS = mmap mmap_file 
LIBS = -lpthread
all: ${SRCS}

CC = gcc
CFLAGS = -O0 -Wall

%:%.c
	${CC} ${CFLAGS} -o $@ $<  ${LIBS} 

clean:
	rm -f ${SRCS} *.o *.s 
