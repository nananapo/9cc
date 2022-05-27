NAME=prpr
TEST_SCRIPT=test.sh

CC=gcc
CFLAGS=-std=c11 -g -static

SRCS=${wildcard *.c}
OBJS=${SRCS:.c=.o}

${NAME}: ${OBJS}
	$(CC) -g -o $(NAME) $(OBJS) $(LDFLAGS)

test: $(NAME)
	./$(TEST_SCRIPT)

clean:
	rm -rf ${OBJS} *~ tmp* *dSYM a.out

fclean: clean
	rm -rf ${NAME}

re: fclean ${NAME}

.PHONEY: test clean fclean re
