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
	my_putchar(c); pline();
	dint((int)c);
	pptrv((void *)c); pline();
	
	i = 424242;
	my_putchar((char)i); pline();
	dint(i);
	pptrv((void *)c); pline();

	p = (void *)1000;
	my_putchar((char)p); pline();
	dint((int) p);
	pptrv(p); pline();
}
