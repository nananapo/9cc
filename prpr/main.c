#include "prlib.h"
#include <stdio.h>
#include <string.h>

int	main(int argc, char **argv)
{
	char	*str;
	char	*filename;
	int		len;
	Token	*tok;
	Node	*node;

	if (argc < 2)
		error("no input file");
	filename = argv[1];
	len = strlen(argv[1]);
	str = read_file(strndup(filename, len));
	if (str == NULL)
		return (0);
	tok = tokenize(str);
	node = parse(tok);
}
