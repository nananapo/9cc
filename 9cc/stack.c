#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

static Stack	*create_stack(void *data)
{
	Stack	*stack;

	stack = (Stack *)malloc(sizeof(Stack));
	stack->data = data;
	stack->prev = NULL;
	return (stack);
}

void	stack_push(Stack **stack, void *data)
{
	Stack	*tmp;

	tmp = create_stack(data);
	tmp->prev = *stack;
	*stack = tmp;
}

void	*stack_pop(Stack **stack)
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

void	*stack_peek(Stack *stack)
{
	if (stack != NULL)
		return (stack->data);
	return (NULL);
}
