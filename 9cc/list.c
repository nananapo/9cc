#include <stdlib.h>
#include <stdbool.h>

t_linked_list	*linked_list_new(listcmp cmp)
{
	t_linked_list	*tmp;

	tmp = (t_linked_list *)malloc(sizeof(t_linked_list));
	tmp->sentinel = (t_linked_list_elem *)malloc(sizeof(t_linked_list_elem));
	tmp->sentinel->next = tmp->sentinel;
	tmp->sentinel->prev = tmp->sentinel;
	tmp->sentinel->is_sentinel = true;
	tmp->cmp = cmp;
	return (tmp);
}

void	linked_list_insert(t_linked_list *list, void *value)
{
	t_linked_list_elem	*elem;

	elem = (t_linked_list_elem *)malloc(sizeof(t_linked_list_elem));
	elem->value = value;
	elem->is_sentinel = false;
	elem->next = list->sentinel->next;
	list->sentinel->next->prev = elem;
	list->sentinel->next = elem;
	elem->prev = list->sentinel;
}

void	linked_list_insert_tail(t_linked_list *list, void *value)
{
	t_linked_list_elem	*elem;

	elem = (t_linked_list_elem *)malloc(sizeof(t_linked_list_elem));
	elem->value = value;
	elem->is_sentinel = false;
	elem->next = list->sentinel;
	elem->prev = list->sentinel->prev;
	list->sentinel->prev->next = elem;
	list->sentinel->prev = elem;
}

t_list_elem	*linked_list_search(t_linked_list *list, void *value)
{
	t_linked_list_elem	*tmp;

	tmp = list->sentinel->next;
	while (!tmp->is_sentinel)
	{
		if (list->cmp(tmp->value, value) == 0)
			return  (tmp);
		tmp = tmp->next;
	}
	return (NULL);
}

void	*linked_list_delete(t_linked_list *list, t_linked_list_elem *elem)
{
	elem->prev->next = elem->next;
	elem->next->prev = elem->prev;
	free(elem);
}

