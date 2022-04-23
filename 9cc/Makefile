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
	rm -rf $(OBJS) *~ tmp* *dSYM

fclean: clean
	rm -rf $(NAME)

re: fclean $(NAME)

.PHONY: test clean fclean re
