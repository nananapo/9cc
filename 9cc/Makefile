CC=gcc
CFLAGS=-std=c11 -g

9cc: 9cc.c

test: 9cc
	./test.sh

clean:
	rm -rf 9cc *.dSYM *.o *~ tmp*

.PHONEY: test clean
