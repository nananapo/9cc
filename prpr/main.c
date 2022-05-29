#include "prlib.h"
#include <stdio.h>
#include <string.h>

int	main(int argc, char **argv)
{
	if (argc < 2)
		error("no input file");
	
	char	*str;
	char	*filename;
	int		len;
	Token	*tok;

	filename = argv[1];
	len = strlen(argv[1]);
	str = read_file(strndup(filename, len));
	if (str == NULL)
		return ;
	tok = parse(str);
}
