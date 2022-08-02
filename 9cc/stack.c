#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

static t_stack	*create_stack(void *data)
{
	t_stack	*stack;

	stack = (t_stack *)malloc(sizeof(t_stack));
	stack->data = data;
	stack->prev = NULL;
	return (stack);
}

void	stack_push(t_stack **stack, void *data)
{
	t_stack	*tmp;

	tmp = create_stack(data);
	tmp->prev = *stack;
	*stack = tmp;
}

void	*stack_pop(t_stack **stack)
{
	void	*tmp;

	if (*stack == NULL)
	{
		fprintf(stderr, "failed to pop stack");
		return (NULL);
	}

	tmp = (*stack)->data;
	*stack = (*stack)->prev;
	return (tmp);
}

void	*stack_peek(t_stack *stack)
{
	if (stack != NULL)
		return (stack->data);
	return (NULL);
}
