int my_putchar(char c);
int my_putstr(char *c, int n);
int dint(int a);
void pptrv(void *ptr);
int pline(void);

int	main(void)
{
	char	c;
	int		i;
	void	*p;

	c = 42;
	my_putchar((char)c); pline();
	
	i = 424242;
	dint((int)i);

	p = (void *)1000;
	pptrv((void *)p); pline();
}
