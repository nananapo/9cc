#include <ctype.h>

int my_print(char *str);
int	pint(int i);

void	myput(char *name, int i, int j)
{
	my_print(name);
	my_print(" ");
	pint(i);
	my_print(" ");
	pint(j);
	my_print("\n");
}

int	main(void)
{
	int	i;

	for (i = 0; i < 256; i=i+1)
		myput("isalnum", i, isalnum(i) == 0);
	for (i = 0; i < 256; i=i+1)
		myput("isalpha", i, isalpha(i) == 0);
	for (i = 0; i < 256; i=i+1)
		myput("isblank", i, isblank(i) == 0);
	for (i = 0; i < 256; i=i+1)
		myput("iscntrl", i, iscntrl(i) == 0);
	for (i = 0; i < 256; i=i+1)
		myput("isdigit", i, isdigit(i) == 0);
	for (i = 0; i < 256; i=i+1)
		myput("isgraph", i, isgraph(i) == 0);
	for (i = 0; i < 256; i=i+1)
		myput("islower", i, islower(i) == 0);
	for (i = 0; i < 256; i=i+1)
		myput("isprint", i, isprint(i) == 0);
	for (i = 0; i < 256; i=i+1)
		myput("ispunct", i, ispunct(i) == 0);
	for (i = 0; i < 256; i=i+1)
		myput("isspace", i, isspace(i) == 0);
	for (i = 0; i < 256; i=i+1)
		myput("isupper", i, isupper(i) == 0);
	for (i = 0; i < 256; i=i+1)
		myput("isxdigit", i, isxdigit(i) == 0);
	for (i = 0; i < 256; i=i+1)
		myput("tolower", i, tolower(i));
	for (i = 0; i < 256; i=i+1)
		myput("toupper", i, toupper(i));
}
