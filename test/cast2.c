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
	dint((int)(char)(int)c);
	pptrv((void *)(int)(char)(int)(void *)c); pline();
	
	i = 424242;
	my_putchar((char)(int)(void *)(int)(char)i); pline();
	dint((int)(void *)(char)(int)(char)i);
	pptrv((void *)(int)(char)(void *)(int)(char)c); pline();

	p = (void *)1000;
	my_putchar((char)(int)(char)(void *)(int)p); pline();
	dint((int)(char)(void *)(int)(char)p);
	pptrv((void *)(int)p); pline();
}
