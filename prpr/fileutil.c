#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 1024

static char	*read_all(int fd);

char	*read_file(char	*name)
{
	int		fd;
	char	*p;

	fd = open(name, O_RDONLY);
	if (fd == -1)
	{
		perror("error");
		return (NULL);
	}
	p = read_all(fd);
	close(fd);
	return (p);
}

static char	*repl(char *source, int source_size, int dest_size)
{
	char	*tmp;
	int		i;

	tmp = calloc(dest_size, sizeof(char));
	i = -1;
	while (++i < source_size)
		tmp[i] = source[i];
	free(source);
	return (tmp);
}

static void	mycopy(char *dest, int index, char *source, int source_size)
{
	int	i;

	i = -1;
	while (++i < source_size)
		dest[index + i] = source[i];
}

static char	*read_all(int fd)
{
	char	*p;
	int		size;
	char	buf[BUF_SIZE];
	int		result;
	int		index;

	size = BUF_SIZE;
	p = calloc(size, sizeof(char));
	index = 0;
	while (1)
	{
		result = read(fd, buf, BUF_SIZE);
		if (result == 0)
			return (p);
		if (result == -1)
			return (NULL);
		if (index + result >= size)
		{
			p = repl(p, size, size * 2);
			size *= 2;
		}
		mycopy(p, index, buf, result);
		index += result;
	}
	return (p);
}
