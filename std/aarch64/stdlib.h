#ifndef STDLIB_H
# define STDLIB_H

# include <stddef.h>

// TODO 実装
void	*malloc(size_t size);
void	*calloc(size_t count, size_t size);

void	free(void *ptr);

int		strtol(char *str, char **endptr, int base);
float	strtof(char *nptr, char ** endptr);
void	exit(int status);

#endif
