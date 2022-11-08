SRCS =	mem.v \
		test.v

MODULE=test

OBJ=mem.o

test:
	iverilog -o ${OBJ} -s ${MODULE} ${SRCS}
	vvp ${OBJ}
