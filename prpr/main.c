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
	int		findex;
	char	*stddir;

	if (argc < 2)
		error("no input file");

	if (strcmp(argv[1], "--stddir") == 0)
	{
		if (argc < 4)
			error("Usage: prpr --stddir [dir] [input file]");
		findex = 3;
		stddir = argv[2];
	}
	else
	{
		findex = 1;
		stddir = "";
	}

	filename = argv[findex];
	len = strlen(filename);
	str = read_file(strndup(filename, len));

	if (str == NULL)
		return (0);

	tok = tokenize(str);

	fprintf(stderr, "# Tokenize End\n");

	node = parse(&tok, 0);

	fprintf(stderr, "# Parse End\n");

	set_currentdir(stddir, filename);

	fprintf(stderr, "# Set Current Dir End\n");

	gen(node);
}
