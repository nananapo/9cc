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
	rm -rf ${OBJS} *~ tmp* *dSYM a.out test1/ test2/ test3/

fclean: clean
	rm -rf ${NAME}

re: fclean ${NAME}

.PHONEY: test clean fclean re
