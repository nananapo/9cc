#include "prlib.h"
#include <stdio.h>
#include <string.h>

int	main(int argc, char **args)
{
	if (argc < 2)
		error("no input file");
	include_file(args[1], strlen(args[1]));
}
