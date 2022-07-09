#include <stdio.h>

int	main(void)
{
	char	*s;
	int		i;

	s = "HelloWorld";
	i = -1;
	while (i < 9)
		printf("%c\n", s[++i]);
}
