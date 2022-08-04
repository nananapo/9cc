#include <stdio.h>

int	main(void)
{
	char	*s;
	int		i;

	s = "HelloWorld";
	s += 1;

	i = -1;
	printf("%d\n", i++);
	printf("%d\n", ++i);

	printf("%s\n", s);

	i = -1;
	printf("%c\n", s[++i]);
	printf("%c\n", s[i++]);
}
