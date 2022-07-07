#ifndef STACK_H
# define STACK_H

typedef struct s_stack
{
	void			*data;
	struct s_stack	*prev;
}	Stack;

void	stack_push(Stack **stack, void *data);
void	*stack_pop(Stack **stack);
void	*stack_peek(Stack *stack);

#endif
