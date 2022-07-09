#ifndef LIST_H
# define LIST_H


typedef struct s_linked_list_elem
{
	struct	s_linked_list_elem	*next;
	struct	s_linked_list_elem	*prev;
	void	*value;
	bool	is_sentinel;
}	t_linked_list_elem;

typedef struct s_linked_list
{
	t_linked_list_elem	*sentinel;
}	t_linked_list;

t_linked_list	*linked_list_new();
void	linked_list_insert(t_linked_list *list, void *value);
void	*linked_list_search(t_linked_list *list, void *value);
void	linked_list_delete(t_linked_list_elem *elem);

#endif

