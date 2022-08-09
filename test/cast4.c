int my_putchar(char c);
int my_putstr(char *c, int n);
int dint(int a);
void pptrv(void *ptr);
int pline(void);

int	main(void)
{
	void	*p;

	p = (void *)1000;
	dint((int)(char)(void *)(int)(char)p);
}
