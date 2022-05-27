#include "prlib.h"
#include <stdio.h>

int	main(int argc, char **args)
{
	char	*str;

	if (argc < 2)
	{
		printf("error: no input file\n");
		return (1);
	}
	str = read_file(*(args + 1));
	if (str == NULL)
		return (1);
	if (process(str) != 1)
		return (1);
}
