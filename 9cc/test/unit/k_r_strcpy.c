int my_print(char *s);
int	dint(int c);

void	strcpy(char *s, char *t)
{
	int	dummy;
	dummy = 0;

	while (*s++ = *t++)
		dummy++;
}

int main(void)
{
 char s[1000];
 char *t;
 int i;

 t = "HelloWorld!\n";

 for (i = 0; i < 1000; i++)
	s[i] = 0;

 my_print(s);
 my_print(t);

 strcpy(s, t);

 my_print(s);
 my_print(t);
}
