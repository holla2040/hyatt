OBJS     =  main.o 
CFLAGS   = -Wall -g
CC       = gcc
INCLUDES = -I.
LIBS     = -lm

main:${OBJS} Makefile
	${CC} ${CFLAGS} ${INCLUDES} -o $@ ${OBJS} ${LIBS}

clean:
	-rm -f *.o core *.core

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<

