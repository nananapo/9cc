NAME=9cc

CC=gcc -fsanitize=address -fno-omit-frame-pointer
CFLAGS=-std=c11 -g -static -Wall -Werror -Wextra

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

$(NAME): $(OBJS)
	$(CC) -g -o $(NAME) $(OBJS) $(LDFLAGS)

test: $(NAME)
	sh test.sh $(arch)

testo: $(NAME)
	sh test_old.sh $(arch)

clean:
	rm -rf $(OBJS) *~ tmp* *dSYM a.out actual.output expected.output test1 test2 test3 debug.dot

fclean: clean
	rm -rf $(NAME)

re: fclean $(NAME)

.PHONY: test clean fclean re
