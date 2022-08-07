NAME=9cc
TEST_SCRIPT=test.sh
TEST_SCRIPT_OLD=test_old.sh

CC=gcc -fsanitize=address -fno-omit-frame-pointer
CFLAGS=-std=c11 -g -static -Wall -Werror -Wextra

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

$(NAME): $(OBJS)
	$(CC) -g -o $(NAME) $(OBJS) $(LDFLAGS)

test: $(NAME)
	env LC_ALL="C" ./$(TEST_SCRIPT)

testo: $(NAME)
	env LC_ALL="C" ./$(TEST_SCRIPT_OLD)

clean:
	rm -rf $(OBJS) *~ tmp* *dSYM a.out actual.output expected.output test1 test2 test3

fclean: clean
	rm -rf $(NAME)

re: fclean $(NAME)

.PHONY: test clean fclean re
