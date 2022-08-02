#ifndef STACK_H
# define STACK_H

typedef struct s_stack
{
	void			*data;
	struct s_stack	*prev;
}	t_stack;

void	stack_push(t_stack **stack, void *data);
void	*stack_pop(t_stack **stack);
void	*stack_peek(t_stack *stack);

#endif
