#include <stdio.h>

typedef struct s_stack
{
	void			*data;
	struct s_stack	*prev;
}	Stack;

int	main(void)
{
	Stack	s;

	printf("sizeof	: %d\n", sizeof(Stack));
	printf("data	: %lu\n", (void *)&s.data - (void *)&s);
	printf("prev	: %lu\n", (void *)&s.prev - (void *)&s);
}
