NAME=prpr
TEST_SCRIPT=test.sh

CC=gcc
#-fsanitize=address -fno-omit-frame-pointer
CFLAGS=-std=c11 -g -static -Wall -Werror -Wextra

SRCS=${wildcard *.c}
OBJS=${SRCS:.c=.o}

${NAME}: ${OBJS}
	$(CC) -g -o $(NAME) $(OBJS) $(LDFLAGS)

clean:
	rm -rf ${OBJS} *~ tmp* *dSYM a.out

fclean: clean
	rm -rf ${NAME}

re: fclean ${NAME}

.PHONEY: test clean fclean re
