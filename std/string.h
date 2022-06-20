#ifndef STRING_H
# define STRING_H

# include <stddef.h>
# include <stdlib.h>

// TODO restrict, const
void	*memcpy(void *s1, void *s2, size_t n);
void	*memmove(void *s1, void *s2, size_t n);
char	*strcpy(char *s1, char *s2);
char	*strncpy(char *s1, char *s2, size_t n);
char	*strcat(char *s1, char *s2);
char	*strncat(char *s1, char *s2, size_t n);
int	memcmp(void *s1, void *s2, size_t n);
int	strcmp(char *s1, char *s2);
int	strcoll(char *s1, char *s2);
int	strncmp(char *s1, char *s2, size_t n);
int	strxfrm(char *s1, char *s2, size_t n);
void	*memchr(void *s, int c, size_t n);
char	*strchr(char *s, int c);
size_t	strcspn(char *s1, char *s2);
char	*strpbrk(char *s1, char *s2);
char	*strrchr(char *s, int c);
size_t	strspn(char *s1, char *s2);
char	*strstr(char *s1, char *s2);
char	*strtok(char *s1, char *s2);
char	*memset(void *s, int c, size_t n);
char	*strerror(int errnum);
size_t	strlen(char *s);

void	*memcpy(void *s1, void *s2, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		*((char *)s1 + i) = *((char *)s2 + i);
		i = i + 1;
	}
	return (s1);
}

void	*memmove(void *s1, void *s2, size_t n)
{
	void	*tmp;

	tmp = malloc(n);
	memcpy(tmp, s2, n);
	memcpy(s1, tmp, n);
	free(tmp);
	return (s1);
}

char	*strcpy(char *s1, char *s2)
{
	strncpy(s1, s2, strlen(s2) + 1);
	return (s1);
}

char	*strncpy(char *s1, char *s2, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n && s2[i] != '\0')
	{
		s1[i] = s2[i];
		i = i + 1;
	}
	while (i < n)
	{
		s1[i] = '\0';
		i = i + 1;
	}
	return (s1);
}

char	*strcat(char *s1, char *s2)
{
	strncat(s1, s2, strlen(s2));
	return (s1);
}

char	*strncat(char *s1, char *s2, size_t n)
{
	size_t	i;
	char	*tmp;

	tmp = s1;
	while (*tmp != '\0')
		tmp = tmp + 1;
	i = 0;
	while (i < n && s2[i] != '\0')
	{
		tmp[i] = s2[i];
		i = i + 1;
	}
	tmp[i] = '\0';
	return (s1);
}

int	memcmp(void *s1, void *s2, size_t n)
{
	int	i;

	i = 0;
	while (i < n && *((char *)s1 + i) == *((char *)s2 + i))
		i = i + 1;
	if (i == n)
		return (0);
	return (*((char *)s1 + i) - *((char *)s2 + i));
}

int	strcmp(char *s1, char *s2)
{
	int	i;

	i = 0;
	while (s1[i] == s2[i] && s1[i] != '\0')
		i = i + 1;
	if (s1[i] == s2[i])
		return (0);
	return (s1[i] - s2[i]);
}

int	strcoll(char *s1, char *s2)
{
	// TODO 未実装
	return (strcmp(s1, s2));
}

int	strncmp(char *s1, char *s2, size_t n)
{
	int	i;

	i = 0;
	while (i < n && s1[i] == s2[i] && s1[i] != '\0')
		i = i + 1;
	if (i == n)
		return (0);
	return (s1[i] - s2[i]);
}

int	strxfrm(char *s1, char *s2, size_t n)
{
	// TODO よくわからないので未実装
	return (0);
}

// TODO unsigned charにconvertしないといけない
void	*memchr(void *s, int c, size_t n)
{
	int		i;
	char	c2;

	c2 = (char) c;
	i = 0;
	while (i < n)
	{
		if (*((char *)s + i) == c)
			return (s + i);
		i = i + 1;
	}
	return (NULL);
}

char	*strchr(char *s, int c)
{
	int		i;
	char	c2;

	c2 = (char) c;
	i = 0;
	while (i < n)
	{
		if (s[i] == c2)
			return (s + i);
		i = i + 1;
	}
	return (NULL);
}

size_t	strcspn(char *s1, char *s2)
{
	int	i;
	int	j;

	i = 0;
	while (s1[i] != '\0')
	{
		j = 0;
		while (s2[j] != '\0')
		{
			if (s1[i] == s2[j])
				return (i);
			j = j + 1;
		}
		i = i + 1;
	}
	return (i);
}

char	*strpbrk(char *s1, char *s2)
{
	int	i;

	i = strcspn(s1, s2);
	if (i == NULL)
		return (NULL);
	return (s1 + i);
}

char	*strrchr(char *s, int c)
{
	int		i;
	char	c2;

	c2 = (char) c;
	i = strlen(s);
	while (i >= 0)
	{
		if (s[i] == c2)
			return (s + i);
		i = i - 1;
	}
	return (NULL);
}

size_t	strspn(char *s1, char *s2)
{
	int	i;

	i = 0;
	while (s1[i] != '\0')
	{
		if (strchr(s2, s1[i]) == NULL)
			return (i);
		i = i + 1;
	}
	return (i);
}

char	*strstr(char *s1, char *s2)
{
	int	i;
	int	j;

	if (strlen(s2) == 0)
		return (s1);
	i = 0;
	while (s1[i] != '\0')
	{
		j = 0;
		while (s2[j] != '\0' && s1[i + j] != '\0' && s1[i + j] == s2[j])
			j = j + 1;
		if (s2[j] == '\0')
			return (s1 + i);
		i = i + 1;
	}
	return (NULL);
}

char	*strtok(char *s1, char *s2)
{
	// TODO 未実装
	return (NULL);
}

char	*memset(void *s, int c, size_t n)
{
	int		i;
	char	c2;

	c2 = (char)c;
	i = 0;
	while (i < n)
	{
		*((char *)s + i) = c2;
		i = i + 1;
	}
	return (s);
}

char	*strerror(int errnum)
{
	// 未実装
	return (NULL);
}

size_t	strlen(char *s)
{
	int	i;
	
	i = 0;
	while (s[i] != '\0')
		i = i + 1;
	return (i);
}

#endif
