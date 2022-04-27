NAME=9cc
TEST_SCRIPT=test.sh

CC=gcc
CFLAGS=-std=c11 -g -static

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

$(NAME): $(OBJS)
	$(CC) -o $(NAME) $(OBJS) $(LDFLAGS)

test: $(NAME)
	./$(TEST_SCRIPT)

clean:
	rm -rf $(OBJS) *~ tmp* *dSYM a.out

fclean: clean
	rm -rf $(NAME)

re: fclean $(NAME)

sub: ${NAME}
	./9cc "`cat test/hey.c`" > tmp.s
	${CC} -o tmp tmp.s test/print.c
	./tmp

sub2: ${NAME}
	./9cc "`cat test/hey2.c`" > tmp2.s
	${CC} -o tmp2 tmp2.s test/print.c
	./tmp2
.PHONY: test clean fclean re
